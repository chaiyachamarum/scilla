/*
 * Copyright (C) 2019 Zilliqa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <cassert>
#include <cstring>

#include "c_schnorr.h"
#include "Schnorr.h"

extern "C" {

// Generate a private/public key pair.
// Memory must already be allocated by caller.
bool genKeyPair_Z(RawBytes_Z* privKey, RawBytes_Z* pubKey)
{
    try {

    // Get key pair from C++ lib.
    std::pair<PrivKey, PubKey> kpair = Schnorr::GenKeyPair();

    // Make sure we can pass on the result back.
    std::vector<unsigned char> privK, pubK;
    kpair.first.Serialize(privK, 0);
    kpair.second.Serialize(pubK, 0);
    if (privKey->len != privK.size())
        err_abort("Schnorr::genKeyPair_Z: Incorrect memory allocated for "
                  "private key");
    if (pubKey->len != pubK.size())
        err_abort(
            "Schnorr::genKeyPair_Z: Incorrect memory allocated for public key");

    // Pass on the result.
    std::memcpy(privKey->data, privK.data(), privK.size());
    std::memcpy(pubKey->data, pubK.data(), pubK.size());

    } catch (...) {
        return false;
    }

    return true;
}

// Sign message with privKey/pubKey. Memory for signature must be allocated by caller.
bool sign_Z(const RawBytes_Z* privKey, const RawBytes_Z* pubKey,
            const RawBytes_Z* message, RawBytes_Z* signature)
{
    try {

    std::vector<unsigned char> privK(privKey->len), pubK(pubKey->len),
        M(message->len), S;

    if (privKey->len != privkey_len)
        err_abort("Schnorr::sign_Z: Incorrect memory allocated for "
                  "private key");
    if (pubKey->len != pubkey_len)
        err_abort("Schnorr::sign_Z: Incorrect memory allocated for public key");

    // Copy inputs to vectors for use by Schnorr.
    std::memcpy(privK.data(), privKey->data, privKey->len);
    std::memcpy(pubK.data(), pubKey->data, pubKey->len);
    std::memcpy(M.data(), message->data, message->len);

    PrivKey keyPriv(privK, 0);
    PubKey keyPub(pubK, 0);
    Signature sig;

    // Sign the message.
    Schnorr::Sign(M, keyPriv, keyPub, sig);
    // Extract signature into byte array.
    sig.Serialize(S, 0);

    if ((int)S.size() != signature->len)
        err_abort("Schnorr::size_Z: Incorrect memory allocated for signature");

    // Copy the results for use by caller.
    std::memcpy(signature->data, S.data(), S.size());

    } catch (...) {
        return false;
    }

    return true;
}

// Verify message with signature and public key of signer
bool verify_Z(const RawBytes_Z* pubKey, const RawBytes_Z* message,
             RawBytes_Z* signature, int *res)
{
    try {

    std::vector<unsigned char> pubK(pubKey->len), M(message->len),
        S(signature->len);

    if (pubKey->len != pubkey_len)
        err_abort(
            "Schnorr::verify_Z: Incorrect memory allocated for public key");
    if (signature->len != signature_len)
        err_abort(
            "Schnorr::verify_Z: Incorrect memory allocated for signature");

    // Copy inputs to vectors for use by Schnorr.
    std::memcpy(pubK.data(), pubKey->data, pubKey->len);
    std::memcpy(M.data(), message->data, message->len);
    std::memcpy(S.data(), signature->data, signature->len);

    PubKey keyPub(pubK, 0);
    Signature sig(S, 0);

    // Sign the message.
    if (Schnorr::Verify(M, sig, keyPub))
        *res = 1;
    else
        *res = 0;

    } catch (...) {
        return false;
    }

    return true;
}
}
