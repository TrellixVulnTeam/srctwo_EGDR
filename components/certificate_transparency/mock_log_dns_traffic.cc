// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/certificate_transparency/mock_log_dns_traffic.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include "base/big_endian.h"
#include "base/numerics/safe_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "base/sys_byteorder.h"
#include "base/test/test_timeouts.h"
#include "net/dns/dns_client.h"
#include "net/dns/dns_protocol.h"
#include "net/dns/dns_util.h"
#include "net/socket/socket_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace certificate_transparency {

namespace {

// This is used for the last mock socket response as a sentinel to prevent
// trying to read more data than expected.
const net::MockRead kNoMoreData(net::SYNCHRONOUS, net::ERR_UNEXPECTED, 2);

// Necessary to expose SetDnsConfig for testing.
class DnsChangeNotifier : public net::NetworkChangeNotifier {
 public:
  static void SetInitialDnsConfig(const net::DnsConfig& config) {
    net::NetworkChangeNotifier::SetInitialDnsConfig(config);
  }

  static void SetDnsConfig(const net::DnsConfig& config) {
    net::NetworkChangeNotifier::SetDnsConfig(config);
  }
};

// Always return min, to simplify testing.
// This should result in the DNS query ID always being 0.
int FakeRandInt(int min, int max) {
  return min;
}

bool CreateDnsTxtRequest(base::StringPiece qname, std::vector<char>* request) {
  std::string encoded_qname;
  if (!net::DNSDomainFromDot(qname, &encoded_qname)) {
    // |qname| is an invalid domain name.
    return false;
  }

  // DNS query section:
  // N bytes - qname
  // 2 bytes - record type
  // 2 bytes - record class
  // Total = N + 4 bytes
  const size_t query_section_size = encoded_qname.size() + 4;

  request->resize(sizeof(net::dns_protocol::Header) + query_section_size);
  base::BigEndianWriter writer(request->data(), request->size());

  // Header
  net::dns_protocol::Header header = {};
  header.flags = base::HostToNet16(net::dns_protocol::kFlagRD);
  header.qdcount = base::HostToNet16(1);

  if (!writer.WriteBytes(&header, sizeof(header)) ||
      !writer.WriteBytes(encoded_qname.data(), encoded_qname.size()) ||
      !writer.WriteU16(net::dns_protocol::kTypeTXT) ||
      !writer.WriteU16(net::dns_protocol::kClassIN)) {
    return false;
  }

  if (writer.remaining() != 0) {
    // Less than the expected amount of data was written.
    return false;
  }

  return true;
}

bool CreateDnsTxtResponse(const std::vector<char>& request,
                          base::StringPiece answer,
                          std::vector<char>* response) {
  // DNS answers section:
  // 2 bytes - qname pointer
  // 2 bytes - record type
  // 2 bytes - record class
  // 4 bytes - time-to-live
  // 2 bytes - size of answer (N)
  // N bytes - answer
  // Total = 12 + N bytes
  const size_t answers_section_size = 12 + answer.size();
  constexpr uint32_t ttl = 86400;  // seconds

  response->resize(request.size() + answers_section_size);
  std::copy(request.begin(), request.end(), response->begin());

  // Modify the header
  net::dns_protocol::Header* header =
      reinterpret_cast<net::dns_protocol::Header*>(response->data());
  header->ancount = base::HostToNet16(1);
  header->flags |= base::HostToNet16(net::dns_protocol::kFlagResponse);

  // The qname is at the start of the query section (just after the header).
  const uint8_t qname_ptr = sizeof(*header);

  // Write the answer section
  base::BigEndianWriter writer(response->data() + request.size(),
                               response->size() - request.size());
  if (!writer.WriteU8(net::dns_protocol::kLabelPointer) ||
      !writer.WriteU8(qname_ptr) ||
      !writer.WriteU16(net::dns_protocol::kTypeTXT) ||
      !writer.WriteU16(net::dns_protocol::kClassIN) || !writer.WriteU32(ttl) ||
      !writer.WriteU16(answer.size()) ||
      !writer.WriteBytes(answer.data(), answer.size())) {
    return false;
  }

  if (writer.remaining() != 0) {
    // Less than the expected amount of data was written.
    return false;
  }

  return true;
}

bool CreateDnsErrorResponse(const std::vector<char>& request,
                            uint8_t rcode,
                            std::vector<char>* response) {
  *response = request;
  // Modify the header
  net::dns_protocol::Header* header =
      reinterpret_cast<net::dns_protocol::Header*>(response->data());
  header->ancount = base::HostToNet16(1);
  header->flags |= base::HostToNet16(net::dns_protocol::kFlagResponse | rcode);
  return true;
}

}  // namespace

// A container for all of the data needed for simulating a socket.
// This is useful because Mock{Read,Write}, SequencedSocketData and
// MockClientSocketFactory all do not take ownership of or copy their arguments,
// so it is necessary to manage the lifetime of those arguments. Wrapping all
// of that up in a single class simplifies this.
class MockLogDnsTraffic::MockSocketData {
 public:
  // A socket that expects one write and one read operation.
  MockSocketData(const std::vector<char>& write, const std::vector<char>& read)
      : expected_write_payload_(write),
        expected_read_payload_(read),
        expected_write_(net::SYNCHRONOUS,
                        expected_write_payload_.data(),
                        expected_write_payload_.size(),
                        0),
        expected_reads_{net::MockRead(net::ASYNC,
                                      expected_read_payload_.data(),
                                      expected_read_payload_.size(),
                                      1),
                        kNoMoreData},
        socket_data_(expected_reads_, 2, &expected_write_, 1) {}

  // A socket that expects one write and a read error.
  MockSocketData(const std::vector<char>& write, net::Error error)
      : expected_write_payload_(write),
        expected_write_(net::SYNCHRONOUS,
                        expected_write_payload_.data(),
                        expected_write_payload_.size(),
                        0),
        expected_reads_{net::MockRead(net::ASYNC, error, 1), kNoMoreData},
        socket_data_(expected_reads_, 2, &expected_write_, 1) {}

  // A socket that expects one write and no response.
  explicit MockSocketData(const std::vector<char>& write)
      : expected_write_payload_(write),
        expected_write_(net::SYNCHRONOUS,
                        expected_write_payload_.data(),
                        expected_write_payload_.size(),
                        0),
        expected_reads_{net::MockRead(net::SYNCHRONOUS, net::ERR_IO_PENDING, 1),
                        kNoMoreData},
        socket_data_(expected_reads_, 2, &expected_write_, 1) {}

  ~MockSocketData() {}

  void SetWriteMode(net::IoMode mode) { expected_write_.mode = mode; }
  void SetReadMode(net::IoMode mode) { expected_reads_[0].mode = mode; }

  void AddToFactory(net::MockClientSocketFactory* socket_factory) {
    socket_factory->AddSocketDataProvider(&socket_data_);
  }

 private:
  // This class only supports one write and one read, so just need to store one
  // payload each.
  const std::vector<char> expected_write_payload_;
  const std::vector<char> expected_read_payload_;

  // Encapsulates the data that is expected to be written to a socket.
  net::MockWrite expected_write_;

  // Encapsulates the data/error that should be returned when reading from a
  // socket. The second "expected" read is a sentinel (see |kNoMoreData|).
  net::MockRead expected_reads_[2];

  // Holds pointers to |expected_write_| and |expected_reads_|. This is what is
  // added to net::MockClientSocketFactory to prepare a mock socket.
  net::SequencedSocketData socket_data_;

  DISALLOW_COPY_AND_ASSIGN(MockSocketData);
};

MockLogDnsTraffic::MockLogDnsTraffic() : socket_read_mode_(net::ASYNC) {}

MockLogDnsTraffic::~MockLogDnsTraffic() {}

bool MockLogDnsTraffic::ExpectRequestAndErrorResponse(base::StringPiece qname,
                                                      uint8_t rcode) {
  std::vector<char> request;
  if (!CreateDnsTxtRequest(qname, &request)) {
    return false;
  }

  std::vector<char> response;
  if (!CreateDnsErrorResponse(request, rcode, &response)) {
    return false;
  }

  EmplaceMockSocketData(request, response);
  return true;
}

bool MockLogDnsTraffic::ExpectRequestAndSocketError(base::StringPiece qname,
                                                    net::Error error) {
  std::vector<char> request;
  if (!CreateDnsTxtRequest(qname, &request)) {
    return false;
  }

  EmplaceMockSocketData(request, error);
  return true;
}

bool MockLogDnsTraffic::ExpectRequestAndTimeout(base::StringPiece qname) {
  std::vector<char> request;
  if (!CreateDnsTxtRequest(qname, &request)) {
    return false;
  }

  EmplaceMockSocketData(request);

  // Speed up timeout tests.
  SetDnsTimeout(TestTimeouts::tiny_timeout());

  return true;
}

bool MockLogDnsTraffic::ExpectRequestAndResponse(
    base::StringPiece qname,
    const std::vector<base::StringPiece>& txt_strings) {
  std::string answer;
  for (base::StringPiece str : txt_strings) {
    // The size of the string must precede it. The size must fit into 1 byte.
    answer.insert(answer.end(), base::checked_cast<uint8_t>(str.size()));
    str.AppendToString(&answer);
  }

  std::vector<char> request;
  if (!CreateDnsTxtRequest(qname, &request)) {
    return false;
  }

  std::vector<char> response;
  if (!CreateDnsTxtResponse(request, answer, &response)) {
    return false;
  }

  EmplaceMockSocketData(request, response);
  return true;
}

bool MockLogDnsTraffic::ExpectLeafIndexRequestAndResponse(
    base::StringPiece qname,
    uint64_t leaf_index) {
  return ExpectRequestAndResponse(qname, {base::Uint64ToString(leaf_index)});
}

bool MockLogDnsTraffic::ExpectAuditProofRequestAndResponse(
    base::StringPiece qname,
    std::vector<std::string>::const_iterator audit_path_start,
    std::vector<std::string>::const_iterator audit_path_end) {
  // Join nodes in the audit path into a single string.
  std::string proof =
      std::accumulate(audit_path_start, audit_path_end, std::string());

  return ExpectRequestAndResponse(qname, {proof});
}

void MockLogDnsTraffic::InitializeDnsConfig() {
  net::DnsConfig dns_config;
  // Use an invalid nameserver address. This prevents the tests accidentally
  // sending real DNS queries. The mock sockets don't care that the address
  // is invalid.
  dns_config.nameservers.push_back(net::IPEndPoint());
  // Don't attempt retransmissions - just fail.
  dns_config.attempts = 1;
  // This ensures timeouts are long enough for memory tests.
  dns_config.timeout = TestTimeouts::action_timeout();
  // Simplify testing - don't require random numbers for the source port.
  // This means our FakeRandInt function should only be called to get query
  // IDs.
  dns_config.randomize_ports = false;

  DnsChangeNotifier::SetInitialDnsConfig(dns_config);
}

void MockLogDnsTraffic::SetDnsConfig(const net::DnsConfig& config) {
  DnsChangeNotifier::SetDnsConfig(config);
}

std::unique_ptr<net::DnsClient> MockLogDnsTraffic::CreateDnsClient() {
  return net::DnsClient::CreateClientForTesting(nullptr, &socket_factory_,
                                                base::Bind(&FakeRandInt));
}

template <typename... Args>
void MockLogDnsTraffic::EmplaceMockSocketData(Args&&... args) {
  mock_socket_data_.emplace_back(
      new MockSocketData(std::forward<Args>(args)...));
  mock_socket_data_.back()->SetReadMode(socket_read_mode_);
  mock_socket_data_.back()->AddToFactory(&socket_factory_);
}

void MockLogDnsTraffic::SetDnsTimeout(const base::TimeDelta& timeout) {
  net::DnsConfig dns_config;
  DnsChangeNotifier::GetDnsConfig(&dns_config);
  dns_config.timeout = timeout;
  DnsChangeNotifier::SetDnsConfig(dns_config);
}

}  // namespace certificate_transparency
