// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/remoting/demuxer_stream_adapter.h"

#include <memory>
#include <vector>

#include "base/callback_helpers.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "media/base/decoder_buffer.h"
#include "media/base/demuxer_stream.h"
#include "media/remoting/fake_media_resource.h"
#include "media/remoting/fake_remoter.h"
#include "media/remoting/proto_utils.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::Invoke;
using testing::Return;

namespace media {
namespace remoting {

class MockDemuxerStreamAdapter {
 public:
  MockDemuxerStreamAdapter(
      scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> media_task_runner,
      const std::string& name,
      DemuxerStream* demuxer_stream,
      mojom::RemotingDataStreamSenderPtrInfo stream_sender_info,
      mojo::ScopedDataPipeProducerHandle producer_handle)
      : weak_factory_(this) {
    rpc_broker_.reset(
        new RpcBroker(base::Bind(&MockDemuxerStreamAdapter::OnSendMessageToSink,
                                 weak_factory_.GetWeakPtr())));
    demuxer_stream_adapter_.reset(new DemuxerStreamAdapter(
        std::move(main_task_runner), std::move(media_task_runner), name,
        demuxer_stream, rpc_broker_->GetWeakPtr(),
        rpc_broker_->GetUniqueHandle(), std::move(stream_sender_info),
        std::move(producer_handle),
        base::Bind(&MockDemuxerStreamAdapter::OnError,
                   weak_factory_.GetWeakPtr())));

    // Faking initialization with random callback handle to start mojo watcher.
    demuxer_stream_adapter_->Initialize(3);
  }

  ~MockDemuxerStreamAdapter() {
    // Make sure unit tests that did not expect errors did not cause any errors.
    EXPECT_TRUE(errors_.empty());
  }

  int rpc_handle() const { return demuxer_stream_adapter_->rpc_handle(); }

  base::WeakPtr<MockDemuxerStreamAdapter> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  void DoDuplicateInitialize() { demuxer_stream_adapter_->Initialize(999); }

  void TakeErrors(std::vector<StopTrigger>* errors) {
    errors->swap(errors_);
    errors_.clear();
  }

  // Fake to signal that it's in reading state.
  void FakeReadUntil(int read_until_count, int callback_handle) {
    std::unique_ptr<pb::RpcMessage> rpc(new pb::RpcMessage());
    rpc->set_handle(rpc_handle());
    rpc->set_proc(pb::RpcMessage::RPC_DS_READUNTIL);
    auto* read_message = rpc->mutable_demuxerstream_readuntil_rpc();
    read_message->set_callback_handle(
        callback_handle);  // Given an unique callback handle.
    read_message->set_count(read_until_count);  // Request 1 frame

    demuxer_stream_adapter_->OnReceivedRpc(std::move(rpc));
  }
  void OnNewBuffer(const scoped_refptr<DecoderBuffer>& frame) {
    demuxer_stream_adapter_->OnNewBuffer(DemuxerStream::kOk, frame);
  }

  void SignalFlush(bool flush) { demuxer_stream_adapter_->SignalFlush(flush); }

  pb::RpcMessage* last_received_rpc() const { return last_received_rpc_.get(); }

 private:
  void OnSendMessageToSink(std::unique_ptr<std::vector<uint8_t>> message) {
    last_received_rpc_.reset(new pb::RpcMessage());
    CHECK(last_received_rpc_->ParseFromArray(message->data(), message->size()));
  }

  void OnError(StopTrigger stop_trigger) { errors_.push_back(stop_trigger); }

  std::unique_ptr<RpcBroker> rpc_broker_;
  std::unique_ptr<DemuxerStreamAdapter> demuxer_stream_adapter_;
  std::unique_ptr<pb::RpcMessage> last_received_rpc_;

  std::vector<StopTrigger> errors_;

  base::WeakPtrFactory<MockDemuxerStreamAdapter> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(MockDemuxerStreamAdapter);
};

class DemuxerStreamAdapterTest : public ::testing::Test {
 public:
  DemuxerStreamAdapterTest() {}
  ~DemuxerStreamAdapterTest() override = default;

  void SetUpDataPipe() {
    constexpr size_t kDataPipeCapacity = 256;
    demuxer_stream_.reset(new FakeDemuxerStream(true));  // audio.
    const MojoCreateDataPipeOptions data_pipe_options{
        sizeof(MojoCreateDataPipeOptions),
        MOJO_CREATE_DATA_PIPE_OPTIONS_FLAG_NONE, 1, kDataPipeCapacity};
    mojom::RemotingDataStreamSenderPtr stream_sender;
    mojo::ScopedDataPipeProducerHandle producer_end;
    mojo::ScopedDataPipeConsumerHandle consumer_end;
    CHECK_EQ(
        MOJO_RESULT_OK,
        mojo::CreateDataPipe(&data_pipe_options, &producer_end, &consumer_end));

    data_stream_sender_.reset(new FakeRemotingDataStreamSender(
        MakeRequest(&stream_sender), std::move(consumer_end)));
    demuxer_stream_adapter_.reset(new MockDemuxerStreamAdapter(
        message_loop_.task_runner(), message_loop_.task_runner(), "test",
        demuxer_stream_.get(), stream_sender.PassInterface(),
        std::move(producer_end)));
    // DemuxerStreamAdapter constructor posts task to main thread to
    // register MessageReceiverCallback. Therefore it should call
    // RunPendingTasks() to make sure task is executed.
    RunPendingTasks();
  }

  void TearDown() override { base::RunLoop().RunUntilIdle(); }

  void RunPendingTasks() { base::RunLoop().RunUntilIdle(); }

 protected:
  void SetUp() override { SetUpDataPipe(); }

  // TODO(miu): Add separate media thread, to test threading also.
  base::MessageLoop message_loop_;
  std::unique_ptr<FakeDemuxerStream> demuxer_stream_;
  std::unique_ptr<FakeRemotingDataStreamSender> data_stream_sender_;
  std::unique_ptr<MockDemuxerStreamAdapter> demuxer_stream_adapter_;

 private:
  DISALLOW_COPY_AND_ASSIGN(DemuxerStreamAdapterTest);
};

TEST_F(DemuxerStreamAdapterTest, SingleReadUntil) {
  // Read will be called once since it doesn't return frame buffer in the dummy
  // implementation.
  EXPECT_CALL(*demuxer_stream_, Read(_)).Times(1);

  demuxer_stream_adapter_->FakeReadUntil(3, 999);
  RunPendingTasks();
}

TEST_F(DemuxerStreamAdapterTest, MultiReadUntil) {
  // Read will be called once since it doesn't return frame buffer in the dummy
  // implementation, and 2nd one will not proceed when there is ongoing read.
  EXPECT_CALL(*demuxer_stream_, Read(_)).Times(1);

  demuxer_stream_adapter_->FakeReadUntil(1, 100);
  RunPendingTasks();

  demuxer_stream_adapter_->FakeReadUntil(2, 101);
  RunPendingTasks();
}

TEST_F(DemuxerStreamAdapterTest, WriteOneFrameSmallerThanCapacity) {
  // Sends a frame with size 50 bytes, pts = 1 and key frame.
  demuxer_stream_->CreateFakeFrame(50, true, 1 /* pts */);
  demuxer_stream_adapter_->FakeReadUntil(1, 999);
  RunPendingTasks();

  // Checks if it's sent to consumer side and data is correct
  ASSERT_EQ(data_stream_sender_->send_frame_count(), 1U);
  ASSERT_EQ(data_stream_sender_->consume_data_chunk_count(), 1U);
  ASSERT_TRUE(data_stream_sender_->ValidateFrameBuffer(0, 50, true, 1));
  pb::RpcMessage* last_rpc = demuxer_stream_adapter_->last_received_rpc();
  ASSERT_TRUE(last_rpc);
  ASSERT_EQ(last_rpc->proc(), pb::RpcMessage::RPC_DS_READUNTIL_CALLBACK);
  ASSERT_EQ(last_rpc->handle(), 999);
  data_stream_sender_->ResetHistory();
}

TEST_F(DemuxerStreamAdapterTest, WriteOneFrameLargerThanCapacity) {
  // Sends a frame with size 800 bytes, pts = 1 and key frame.
  demuxer_stream_->CreateFakeFrame(800, true, 1 /* pts */);
  demuxer_stream_adapter_->FakeReadUntil(1, 999);
  RunPendingTasks();

  // Checks if it's sent to consumer side and data is correct
  ASSERT_EQ(data_stream_sender_->send_frame_count(), 1U);
  ASSERT_EQ(data_stream_sender_->consume_data_chunk_count(), 4U);
  ASSERT_TRUE(data_stream_sender_->ValidateFrameBuffer(0, 800, true, 1));
  pb::RpcMessage* last_rpc = demuxer_stream_adapter_->last_received_rpc();
  ASSERT_TRUE(last_rpc);
  ASSERT_EQ(last_rpc->proc(), pb::RpcMessage::RPC_DS_READUNTIL_CALLBACK);
  ASSERT_EQ(last_rpc->handle(), 999);
  data_stream_sender_->ResetHistory();
}

TEST_F(DemuxerStreamAdapterTest, SendFrameAndSignalFlushMix) {
  // Sends a frame with size 50 bytes, pts = 1 and key frame.
  demuxer_stream_->CreateFakeFrame(50, true, 1 /* pts */);
  // Issues ReadUntil request with frame count up to 1 (fetch #0).
  demuxer_stream_adapter_->FakeReadUntil(1, 100);
  RunPendingTasks();
  ASSERT_EQ(data_stream_sender_->send_frame_count(), 1U);
  ASSERT_TRUE(data_stream_sender_->ValidateFrameBuffer(0, 50, true, 1));
  pb::RpcMessage* last_rpc = demuxer_stream_adapter_->last_received_rpc();
  ASSERT_TRUE(last_rpc);
  ASSERT_EQ(last_rpc->proc(), pb::RpcMessage::RPC_DS_READUNTIL_CALLBACK);
  ASSERT_EQ(last_rpc->handle(), 100);
  data_stream_sender_->ResetHistory();

  // Sends two frames with size 100 + 150 bytes
  demuxer_stream_->CreateFakeFrame(100, false, 2 /* pts */);
  demuxer_stream_->CreateFakeFrame(150, false, 3 /* pts */);
  // Issues ReadUntil request with frame count up to 3 (fetch #1 and #2).
  demuxer_stream_adapter_->FakeReadUntil(3, 101);
  RunPendingTasks();
  ASSERT_EQ(data_stream_sender_->send_frame_count(), 2U);
  ASSERT_TRUE(data_stream_sender_->ValidateFrameBuffer(0, 100, false, 2));
  ASSERT_TRUE(data_stream_sender_->ValidateFrameBuffer(1, 150, false, 3));
  last_rpc = demuxer_stream_adapter_->last_received_rpc();
  ASSERT_TRUE(last_rpc);
  ASSERT_EQ(last_rpc->proc(), pb::RpcMessage::RPC_DS_READUNTIL_CALLBACK);
  ASSERT_EQ(last_rpc->handle(), 101);
  data_stream_sender_->ResetHistory();

  // Signal flush
  ASSERT_EQ(data_stream_sender_->cancel_in_flight_count(), 0U);
  demuxer_stream_adapter_->SignalFlush(true);
  RunPendingTasks();
  ASSERT_EQ(data_stream_sender_->cancel_in_flight_count(), 1U);

  // ReadUntil request after flush signaling should be ignored.
  demuxer_stream_->CreateFakeFrame(100, false, 4 /* pts */);
  demuxer_stream_->CreateFakeFrame(100, false, 5 /* pts */);
  // Issues ReadUntil request with frame count up to 5 (fetch #3 and #4).
  demuxer_stream_adapter_->FakeReadUntil(5, 102);
  RunPendingTasks();
  ASSERT_EQ(data_stream_sender_->send_frame_count(), 0U);

  // Signal flush done
  demuxer_stream_adapter_->SignalFlush(false);
  RunPendingTasks();
  ASSERT_EQ(data_stream_sender_->cancel_in_flight_count(), 1U);
  data_stream_sender_->ResetHistory();

  // Re-issues ReadUntil request with frame count up to 4 (fetch #3).
  demuxer_stream_adapter_->FakeReadUntil(4, 103);
  RunPendingTasks();
  ASSERT_EQ(data_stream_sender_->send_frame_count(), 1U);
  ASSERT_TRUE(data_stream_sender_->ValidateFrameBuffer(0, 100, false, 4));
  last_rpc = demuxer_stream_adapter_->last_received_rpc();
  ASSERT_TRUE(last_rpc);
  ASSERT_EQ(last_rpc->proc(), pb::RpcMessage::RPC_DS_READUNTIL_CALLBACK);
  ASSERT_EQ(last_rpc->handle(), 103);
  data_stream_sender_->ResetHistory();
}

TEST_F(DemuxerStreamAdapterTest, DuplicateInitializeCausesFatalError) {
  std::vector<StopTrigger> errors;
  demuxer_stream_adapter_->TakeErrors(&errors);
  ASSERT_TRUE(errors.empty());

  demuxer_stream_adapter_->DoDuplicateInitialize();
  demuxer_stream_adapter_->TakeErrors(&errors);
  ASSERT_EQ(1u, errors.size());
  EXPECT_EQ(PEERS_OUT_OF_SYNC, errors[0]);
}

TEST_F(DemuxerStreamAdapterTest, ClosingPipeCausesFatalError) {
  std::vector<StopTrigger> errors;
  demuxer_stream_adapter_->TakeErrors(&errors);
  ASSERT_TRUE(errors.empty());

  // Closes one end of mojo message and data pipes.
  data_stream_sender_.reset();
  RunPendingTasks();  // Allow notification from mojo to propagate.

  demuxer_stream_adapter_->TakeErrors(&errors);
  ASSERT_EQ(1u, errors.size());
  EXPECT_EQ(MOJO_PIPE_ERROR, errors[0]);
}

}  // namesapce remoting
}  // namespace media
