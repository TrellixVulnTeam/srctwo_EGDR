// Test importKey and exportKey for non-PKC algorithms. Only "happy paths" are
// currently tested - those where the operation should succeed.

function run_test() {
    var subtle = crypto.subtle;

    // keying material for algorithms that can use any bit string.
    var rawKeyData = [
        new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]),
        new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24]),
        new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32])
    ];

    // combinations of algorithms, usages, parameters, and formats to test
    var testVectors = [
        {name: "AES-CTR",               legalUsages: ["encrypt", "decrypt"],      extractable: [true, false], formats: ["raw", "jwk"]},
        {name: "AES-CBC",               legalUsages: ["encrypt", "decrypt"],      extractable: [true, false], formats: ["raw", "jwk"]},
        {name: "AES-GCM",               legalUsages: ["encrypt", "decrypt"],      extractable: [true, false], formats: ["raw", "jwk"]},
        {name: "AES-KW",                legalUsages: ["wrapKey", "unwrapKey"],    extractable: [true, false], formats: ["raw", "jwk"]},
        {name: "HMAC", hash: "SHA-1",   legalUsages: ["sign", "verify"],          extractable: [false],       formats: ["raw", "jwk"]},
        {name: "HMAC", hash: "SHA-256", legalUsages: ["sign", "verify"],          extractable: [false],       formats: ["raw", "jwk"]},
        {name: "HMAC", hash: "SHA-384", legalUsages: ["sign", "verify"],          extractable: [false],       formats: ["raw", "jwk"]},
        {name: "HMAC", hash: "SHA-512", legalUsages: ["sign", "verify"],          extractable: [false],       formats: ["raw", "jwk"]},
        {name: "HKDF",                  legalUsages: ["deriveBits", "deriveKey"], extractable: [false],       formats: ["raw"]},
        {name: "PBKDF2",                legalUsages: ["deriveBits", "deriveKey"], extractable: [false],       formats: ["raw"]}
    ];



    // TESTS ARE HERE:
    // Test every test vector, along with all available key data
    testVectors.forEach(function(vector) {
        var algorithm = {name: vector.name};
        if ("hash" in vector) {
            algorithm.hash = vector.hash;
        }

        rawKeyData.forEach(function(keyData) {
            // Generate all combinations of valid usages for testing
            allValidUsages(vector.legalUsages, []).forEach(function(usages) {
                // Try each legal value of the extractable parameter
                vector.extractable.forEach(function(extractable) {
                    vector.formats.forEach(function(format) {
                        var data = keyData;
                        if (format === "jwk") {
                            data = jwkData(keyData, algorithm);
                        }
                        testFormat(format, algorithm, data, keyData.length * 8, usages, extractable);
                    });
                });
            });

        });
    });

    // Test importKey with a given key format and other parameters. If
    // extrable is true, export the key and verify that it matches the input.
    function testFormat(format, algorithm, keyData, keySize, usages, extractable) {
        promise_test(function(test) {
            return subtle.importKey(format, keyData, algorithm, extractable, usages).
            then(function(key) {
                assert_equals(key.constructor, CryptoKey, "Imported a CryptoKey object");
                if (!extractable) {
                    return;
                }

                return subtle.exportKey(format, key).
                then(function(result) {
                    if (format !== "jwk") {
                        assert_true(equalBuffers(keyData, result), "Round trip works");
                    } else {
                        assert_true(equalJwk(keyData, result), "Round trip works");
                    }
                }, function(err) {
                    assert_unreached("Threw an unexpected error: " + err.toString());
                });
            }, function(err) {
                assert_unreached("Threw an unexpected error: " + err.toString());
            });
        }, "Good parameters: " + keySize.toString() + " bits " + parameterString(format, keyData, algorithm, extractable, usages));
    }



    // Helper methods follow:

    // Are two array buffers the same?
    function equalBuffers(a, b) {
        if (a.byteLength !== b.byteLength) {
            return false;
        }

        var aBytes = new Uint8Array(a);
        var bBytes = new Uint8Array(b);

        for (var i=0; i<a.byteLength; i++) {
            if (aBytes[i] !== bBytes[i]) {
                return false;
            }
        }

        return true;
    }

    // Are two Jwk objects "the same"? That is, does the object returned include
    // matching values for each property that was expected? It's okay if the
    // returned object has extra methods; they aren't checked.
    function equalJwk(expected, got) {
        var fields = Object.keys(expected);
        var fieldName;

        for(var i=0; i<fields.length; i++) {
            fieldName = fields[i];
            if (!(fieldName in got)) {
                return false;
            }
            if (expected[fieldName] !== got[fieldName]) {
                return false;
            }
        }

        return true;
    }

    // Build minimal Jwk objects from raw key data and algorithm specifications
    function jwkData(keyData, algorithm) {
        var result = {
            kty: "oct",
            k: byteArrayToUnpaddedBase64(keyData)
        };

        if (algorithm.name.substring(0, 3) === "AES") {
            result.alg = "A" + (8 * keyData.byteLength).toString() + algorithm.name.substring(4);
        } else if (algorithm.name === "HMAC") {
            result.alg = "HS" + algorithm.hash.substring(4);
        }
        return result;
    }

    // Jwk format wants Base 64 without the typical padding at the end.
    function byteArrayToUnpaddedBase64(byteArray){
        var binaryString = "";
        for (var i=0; i<byteArray.byteLength; i++){
            binaryString += String.fromCharCode(byteArray[i]);
        }
        var base64String = btoa(binaryString);

        return base64String.replace(/=/g, "");
    }

    // Want to test every valid combination of usages. Start by creating a list
    // of all non-empty subsets to possible usages.
    function allNonemptySubsetsOf(arr) {
        var results = [];
        var firstElement;
        var remainingElements;

        for(var i=0; i<arr.length; i++) {
            firstElement = arr[i];
            remainingElements = arr.slice(i+1);
            results.push([firstElement]);

            if (remainingElements.length > 0) {
                allNonemptySubsetsOf(remainingElements).forEach(function(combination) {
                    combination.push(firstElement);
                    results.push(combination);
                });
            }
        }

        return results;
    }

    // Return a list of all valid usage combinations, given the possible ones
    // and the ones that are required for a particular operation.
    function allValidUsages(possibleUsages, requiredUsages) {
        var allUsages = [];

        allNonemptySubsetsOf(possibleUsages).forEach(function(usage) {
            for (var i=0; i<requiredUsages.length; i++) {
                if (!usage.includes(requiredUsages[i])) {
                    return;
                }
            }
            allUsages.push(usage);
        });

        return allUsages;
    }

    // Convert method parameters to a string to uniquely name each test
    function parameterString(format, data, algorithm, extractable, usages) {
        var result = "(" +
                        objectToString(format) + ", " +
                        objectToString(data) + ", " +
                        objectToString(algorithm) + ", " +
                        objectToString(extractable) + ", " +
                        objectToString(usages) +
                     ")";

        return result;
    }

    // Character representation of any object we may use as a parameter.
    function objectToString(obj) {
        var keyValuePairs = [];

        if (Array.isArray(obj)) {
            return "[" + obj.map(function(elem){return objectToString(elem);}).join(", ") + "]";
        } else if (typeof obj === "object") {
            Object.keys(obj).sort().forEach(function(keyName) {
                keyValuePairs.push(keyName + ": " + objectToString(obj[keyName]));
            });
            return "{" + keyValuePairs.join(", ") + "}";
        } else if (typeof obj === "undefined") {
            return "undefined";
        } else {
            return obj.toString();
        }

        var keyValuePairs = [];

        Object.keys(obj).sort().forEach(function(keyName) {
            var value = obj[keyName];
            if (typeof value === "object") {
                value = objectToString(value);
            } else if (typeof value === "array") {
                value = "[" + value.map(function(elem){return objectToString(elem);}).join(", ") + "]";
            } else {
                value = value.toString();
            }

            keyValuePairs.push(keyName + ": " + value);
        });

        return "{" + keyValuePairs.join(", ") + "}";
    }

    return; // from run_test
}
