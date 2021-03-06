// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "script.h"

#include <list>

#include "hash.h"
#include "primitives/transaction.h" //TODO: Remove circular dep - move GetWithdrawSpent
#include "streams.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "version.h"

namespace {
inline std::string ValueString(const std::vector<unsigned char>& vch)
{
    if (vch.size() <= 4)
        return strprintf("%d", CScriptNum(vch, false).getint());
    else
        return HexStr(vch);
}
} // anon namespace

using namespace std;

const char* GetOpName(opcodetype opcode)
{
    switch (opcode)
    {
    // push value
    case OP_0                      : return "0";
    case OP_PUSHDATA1              : return "OP_PUSHDATA1";
    case OP_PUSHDATA2              : return "OP_PUSHDATA2";
    case OP_PUSHDATA4              : return "OP_PUSHDATA4";
    case OP_1NEGATE                : return "-1";
    case OP_RESERVED               : return "OP_RESERVED";
    case OP_1                      : return "1";
    case OP_2                      : return "2";
    case OP_3                      : return "3";
    case OP_4                      : return "4";
    case OP_5                      : return "5";
    case OP_6                      : return "6";
    case OP_7                      : return "7";
    case OP_8                      : return "8";
    case OP_9                      : return "9";
    case OP_10                     : return "10";
    case OP_11                     : return "11";
    case OP_12                     : return "12";
    case OP_13                     : return "13";
    case OP_14                     : return "14";
    case OP_15                     : return "15";
    case OP_16                     : return "16";

    // control
    case OP_NOP                    : return "OP_NOP";
    case OP_VER                    : return "OP_VER";
    case OP_IF                     : return "OP_IF";
    case OP_NOTIF                  : return "OP_NOTIF";
    case OP_VERIF                  : return "OP_VERIF";
    case OP_VERNOTIF               : return "OP_VERNOTIF";
    case OP_ELSE                   : return "OP_ELSE";
    case OP_ENDIF                  : return "OP_ENDIF";
    case OP_VERIFY                 : return "OP_VERIFY";
    case OP_RETURN                 : return "OP_RETURN";

    // stack ops
    case OP_TOALTSTACK             : return "OP_TOALTSTACK";
    case OP_FROMALTSTACK           : return "OP_FROMALTSTACK";
    case OP_2DROP                  : return "OP_2DROP";
    case OP_2DUP                   : return "OP_2DUP";
    case OP_3DUP                   : return "OP_3DUP";
    case OP_2OVER                  : return "OP_2OVER";
    case OP_2ROT                   : return "OP_2ROT";
    case OP_2SWAP                  : return "OP_2SWAP";
    case OP_IFDUP                  : return "OP_IFDUP";
    case OP_DEPTH                  : return "OP_DEPTH";
    case OP_DROP                   : return "OP_DROP";
    case OP_DUP                    : return "OP_DUP";
    case OP_NIP                    : return "OP_NIP";
    case OP_OVER                   : return "OP_OVER";
    case OP_PICK                   : return "OP_PICK";
    case OP_ROLL                   : return "OP_ROLL";
    case OP_ROT                    : return "OP_ROT";
    case OP_SWAP                   : return "OP_SWAP";
    case OP_TUCK                   : return "OP_TUCK";

    // splice ops
    case OP_CAT                    : return "OP_CAT";
    case OP_SUBSTR                 : return "OP_SUBSTR";
    case OP_SUBSTR_LAZY            : return "OP_SUBSTR_LAZY";
    case OP_LEFT                   : return "OP_LEFT";
    case OP_RIGHT                  : return "OP_RIGHT";
    case OP_SIZE                   : return "OP_SIZE";

    // bit logic
    case OP_INVERT                 : return "OP_INVERT";
    case OP_AND                    : return "OP_AND";
    case OP_OR                     : return "OP_OR";
    case OP_XOR                    : return "OP_XOR";
    case OP_EQUAL                  : return "OP_EQUAL";
    case OP_EQUALVERIFY            : return "OP_EQUALVERIFY";
    case OP_RESERVED1              : return "OP_RESERVED1";
    case OP_RESERVED2              : return "OP_RESERVED2";

    // numeric
    case OP_1ADD                   : return "OP_1ADD";
    case OP_1SUB                   : return "OP_1SUB";
    case OP_2MUL                   : return "OP_2MUL";
    case OP_2DIV                   : return "OP_2DIV";
    case OP_NEGATE                 : return "OP_NEGATE";
    case OP_ABS                    : return "OP_ABS";
    case OP_NOT                    : return "OP_NOT";
    case OP_0NOTEQUAL              : return "OP_0NOTEQUAL";
    case OP_ADD                    : return "OP_ADD";
    case OP_SUB                    : return "OP_SUB";
    case OP_MUL                    : return "OP_MUL";
    case OP_DIV                    : return "OP_DIV";
    case OP_MOD                    : return "OP_MOD";
    case OP_LSHIFT                 : return "OP_LSHIFT";
    case OP_RSHIFT                 : return "OP_RSHIFT";
    case OP_BOOLAND                : return "OP_BOOLAND";
    case OP_BOOLOR                 : return "OP_BOOLOR";
    case OP_NUMEQUAL               : return "OP_NUMEQUAL";
    case OP_NUMEQUALVERIFY         : return "OP_NUMEQUALVERIFY";
    case OP_NUMNOTEQUAL            : return "OP_NUMNOTEQUAL";
    case OP_LESSTHAN               : return "OP_LESSTHAN";
    case OP_GREATERTHAN            : return "OP_GREATERTHAN";
    case OP_LESSTHANOREQUAL        : return "OP_LESSTHANOREQUAL";
    case OP_GREATERTHANOREQUAL     : return "OP_GREATERTHANOREQUAL";
    case OP_MIN                    : return "OP_MIN";
    case OP_MAX                    : return "OP_MAX";
    case OP_WITHIN                 : return "OP_WITHIN";

    // crypto
    case OP_RIPEMD160              : return "OP_RIPEMD160";
    case OP_SHA1                   : return "OP_SHA1";
    case OP_SHA256                 : return "OP_SHA256";
    case OP_HASH160                : return "OP_HASH160";
    case OP_HASH256                : return "OP_HASH256";
    case OP_CODESEPARATOR          : return "OP_CODESEPARATOR";
    case OP_CHECKSIG               : return "OP_CHECKSIG";
    case OP_CHECKSIGVERIFY         : return "OP_CHECKSIGVERIFY";
    case OP_CHECKMULTISIG          : return "OP_CHECKMULTISIG";
    case OP_CHECKMULTISIGVERIFY    : return "OP_CHECKMULTISIGVERIFY";
    case OP_DETERMINISTICRANDOM    : return "OP_DETERMINISTICRANDOM";
    case OP_CHECKSIGFROMSTACK      : return "OP_CHECKSIGFROMSTACK";
    case OP_CHECKSIGFROMSTACKVERIFY: return "OP_CHECKSIGFROMSTACKVERIFY";

    // expanson
    case OP_NOP1                   : return "OP_NOP1";
    case OP_NOP2                   : return "OP_NOP2";
    case OP_NOP3                   : return "OP_NOP3";
    case OP_NOP6                   : return "OP_NOP6";
    case OP_NOP7                   : return "OP_NOP7";
    case OP_NOP8                   : return "OP_NOP8";
    case OP_NOP9                   : return "OP_NOP9";
    case OP_NOP10                  : return "OP_NOP10";

    // sidechains/withdraw-proofs
    case OP_WITHDRAWPROOFVERIFY    : return "OP_WITHDRAWPROOFVERIFY";
    case OP_REORGPROOFVERIFY       : return "OP_REORGPROOFVERIFY";

    case OP_INVALIDOPCODE          : return "OP_INVALIDOPCODE";

    // Note:
    //  The template matching params OP_SMALLDATA/etc are defined in opcodetype enum
    //  as kind of implementation hack, they are *NOT* real opcodes.  If found in real
    //  Script, just let the default: case deal with them.

    default:
        return "OP_UNKNOWN";
    }
}

unsigned int CScript::GetSigOpCount(bool fAccurate) const
{
    unsigned int n = 0;
    const_iterator pc = begin();
    opcodetype lastOpcode = OP_INVALIDOPCODE;
    while (pc < end())
    {
        opcodetype opcode;
        if (!GetOp(pc, opcode))
            break;
        if (opcode == OP_CHECKSIG || opcode == OP_CHECKSIGVERIFY)
            n++;
        else if (opcode == OP_CHECKMULTISIG || opcode == OP_CHECKMULTISIGVERIFY)
        {
            if (fAccurate && lastOpcode >= OP_1 && lastOpcode <= OP_16)
                n += DecodeOP_N(lastOpcode);
            else
                n += 20;
        }
        lastOpcode = opcode;
    }
    return n;
}

unsigned int CScript::GetSigOpCount(const CScript& scriptSig) const
{
    if (!IsPayToScriptHash())
        return GetSigOpCount(true);

    // This is a pay-to-script-hash scriptPubKey;
    // get the last item that the scriptSig
    // pushes onto the stack:
    const_iterator pc = scriptSig.begin();
    vector<unsigned char> data;
    while (pc < scriptSig.end())
    {
        opcodetype opcode;
        if (!scriptSig.GetOp(pc, opcode, data))
            return 0;
        if (opcode > OP_16)
            return 0;
    }

    /// ... and return its opcount:
    CScript subscript(data.begin(), data.end());
    return subscript.GetSigOpCount(true);
}

bool CScript::IsWithdrawProof() const
{
    // Format is <scriptSig for the <...> script> <...> <proof push>x5 for 10 total pushes
    // The <...> script fragment must match the HASH160(<...>) script fragment in the withdraw lock
    // (note that 3/4 of the pushes are <push>xN <N> to allow for >520 byte pushes)
    // Here we simply check that the script is push-only and has at least 10 pushes.
    // The output must be OP_IF <partial proof push(es)> HASH160(<...>) OP_REORGPROOFVERIFY OP_ELSE <N> OP_CHECKSEQUENCEVERIFY <p2sh script> OP_ENDIF
    const_iterator pc = begin();
    opcodetype opcode;
    uint32_t push_count = 0;
    while (pc < end())
    {
        if (!GetOp(pc, opcode))
            return false;
        if (opcode > OP_16 || opcode == OP_RESERVED)
            return false;
        push_count++;
    }
    return push_count >= 10;
}

bool CScript::IsWithdrawOutput() const
{
    // Format is OP_IF lockTxHeight <lockTxHash> nlocktxOut [<workAmount>] reorgBounty Hash160(<...>) <genesisHash> OP_REORGPROOFVERIFY OP_ELSE withdrawLockTime OP_CHECKSEQUENCEVERIFY << OP_DROP << OP_HASH160 << p2shWithdrawDest << OP_EQUAL << OP_ENDIF
    // In order for any OP_REORGPROOFVERIFY opcode to be executed,
    // this function must return true.
    // Note that, as a result, an OP_REORGPROOFVERIFY may execute even if not
    // all pushes are in their minimal encoding, however an
    // OP_WITHDRAWPROOFVERIFY check will not allow such an output.
    const_iterator pc = begin();
    vector<unsigned char> data;
    opcodetype opcode;

    if (!GetOp(pc, opcode, data) || opcode != OP_IF)
        return false;

    if (!GetOp(pc, opcode, data) || opcode == OP_RESERVED || opcode > OP_16)
        return false;

    if (!GetOp(pc, opcode, data) || data.size() != 32)
        return false;

    if (!GetOp(pc, opcode, data) || opcode == OP_RESERVED || opcode > OP_16)
        return false;

#define FEDERATED_PEG_SIDECHAIN_ONLY
#ifndef FEDERATED_PEG_SIDECHAIN_ONLY
    if (!GetOp(pc, opcode, data) || opcode == OP_RESERVED || opcode > OP_16)
        return false;
#endif

    if (!GetOp(pc, opcode, data) || opcode == OP_RESERVED || opcode > OP_16 || data.size() > 8)
        return false;

    if (!GetOp(pc, opcode, data) || data.size() != 20)
        return false;

    if (!GetOp(pc, opcode, data) || data.size() != 32)
        return false;

    if (!GetOp(pc, opcode, data) || opcode != OP_REORGPROOFVERIFY)
        return false;

    if (!GetOp(pc, opcode, data) || opcode != OP_ELSE)
        return false;

    if (!GetOp(pc, opcode, data) || opcode == OP_RESERVED || opcode > OP_16)
        return false;

    if (!GetOp(pc, opcode, data) || opcode != OP_CHECKSEQUENCEVERIFY)
        return false;

    if (!GetOp(pc, opcode, data) || opcode != OP_DROP)
        return false;

    if (!GetOp(pc, opcode, data) || opcode != OP_HASH160)
        return false;

    if (!GetOp(pc, opcode, data) || data.size() != 20)
        return false;

    if (!GetOp(pc, opcode, data) || opcode != OP_EQUAL)
        return false;

    if (!GetOp(pc, opcode, data) || opcode != OP_ENDIF)
        return false;

    if (GetOp(pc, opcode, data))
        return false;

    return true;
}

CAmount CScript::GetFraudBounty() const
{
    assert(IsWithdrawOutput());
    const_iterator pc = begin();
    vector<unsigned char> data;
    opcodetype opcode;

    assert(GetOp(pc, opcode, data));
    assert(GetOp(pc, opcode, data));
    assert(GetOp(pc, opcode, data));
    assert(GetOp(pc, opcode, data));

#ifndef FEDERATED_PEG_SIDECHAIN_ONLY
    assert(GetOp(pc, opcode, data));
#endif

    assert(GetOp(pc, opcode, data));
    if (opcode <= OP_16 && opcode >= OP_1)
        data.push_back(opcode - OP_1 + 1);
    else if (opcode == OP_1NEGATE)
        data.push_back(0x81);

    // data.size() <= 8 defined as a part of IsWithdrawOutput()
    return CScriptNum(data, false, 8).getint64();
}

bool CScript::IsWithdrawLock(const uint256 hashGenesisBlock, bool fRequireDestination, bool fRequireToUs) const
{
    // Locks look like [<chaindest> OP_DROP] <genesishash> HASH160(<...>) OP_WITHDRAWPROOFVERIFY
    // Note that <...> can be any script chunk and sidechains MUST verify it is some
    // expected value before accepting the transfer.
    // Like IsWithdrawOutput, this function must return true for an
    // OP_WITHDRAWPROOFVERIFY opcode to execute.
    // However, unlike IsWithdrawOutput, we require all pushes be in their
    // minimal form, to make inspection of withdraw locks a purely
    // byte-matching affair.
    const_iterator pc = begin();
    vector<unsigned char> data;
    opcodetype opcode;

    if (!GetOp(pc, opcode, data))
        return false;
    if (opcode == 24 && data.size() == 24) { // 4 byte type + 20 byte destination is suggested
        if (fRequireToUs && (data[0] != 'P' || data[1] != '2' || data[2] != 'S' || data[3] != 'H'))
            return false;

        if (!GetOp(pc, opcode, data) || opcode != OP_DROP || data.size() != 0)
            return false;

        if (!GetOp(pc, opcode, data))
            return false;
    } else if (fRequireDestination)
        return false;

    if (opcode != 32 || data.size() != 32)
        return false;

    if (fRequireToUs && uint256(data) != hashGenesisBlock)
        return false;

    if (!GetOp(pc, opcode, data) || data.size() != 20)
        return false;

    if (fRequireToUs) {
        vector<unsigned char> vExpectedHash(20);
        //TODO: Require some fraud bounty
        CScript expectedScript = CScript() << OP_DROP << CScriptNum(144) << OP_LESSTHANOREQUAL;
        CHash160().Write(begin_ptr(expectedScript), expectedScript.size()).Finalize(begin_ptr(vExpectedHash));
        if (data != vExpectedHash)
            return false;
    }

    if (!GetOp(pc, opcode, data) || opcode != OP_WITHDRAWPROOFVERIFY || data.size() != 0)
        return false;

    if (fRequireToUs && GetOp(pc, opcode))
        return false;

    return true;
}

uint256 CScript::GetWithdrawLockGenesisHash() const
{
    assert(IsWithdrawLock(0));

    const_iterator pc = begin();
    opcodetype opcode;
    vector<unsigned char> vchgenesishash;

    assert(GetOp(pc, opcode, vchgenesishash));
    if (vchgenesishash.size() != 32) {
        assert(GetOp(pc, opcode) && opcode == OP_DROP);
        assert(GetOp(pc, opcode, vchgenesishash));
    }
    assert(vchgenesishash.size() == 32);
    return uint256(vchgenesishash);
}

static bool PopWithdrawPush(vector<vector<unsigned char> >& pushes, vector<unsigned char> *read=NULL) {
    if (pushes.empty())
        return false;
    int pushCount = CScriptNum(pushes.back(), false).getint();
    pushes.pop_back();
    if (pushCount < 0 || pushCount > 2000 || pushes.size() < size_t(pushCount))
        return false;
    for (int i = pushCount; i > 0; i--) {
        if (i != 1 && pushes[pushes.size() - i].size() != 520)
            return false;
        if (read != NULL) {
            const vector<unsigned char> &push = pushes[pushes.size() - i];
            read->insert(read->end(), push.begin(), push.end());
        }
    }
    for (int i = 0; i < pushCount; i++)
        pushes.pop_back();
    return true;
}

COutPoint CScript::GetWithdrawSpent() const
{
    assert(IsWithdrawProof());

    try {
        const_iterator pc = begin();
        opcodetype opcode;

        // We have to read the script from back-to-front, so we stack-ize it
        vector<vector<unsigned char> > pushes;
        pushes.reserve(10);
        while (pc < end()) {
            pushes.push_back(vector<unsigned char>());
            assert(GetOp(pc, opcode, pushes.back()));
            if (opcode <= OP_16 && opcode >= OP_1)
                pushes.back().push_back(opcode - OP_1 + 1);
            else if (opcode == OP_1NEGATE)
                pushes.back().push_back(0x81);
        }

#ifndef FEDERATED_PEG_SIDECHAIN_ONLY
        // SPV proof
        if (!PopWithdrawPush(pushes))
            return COutPoint();
#endif

        // Coinbase tx
        if (!PopWithdrawPush(pushes))
            return COutPoint();

        if (pushes.empty())
            return COutPoint();
        int ntxOut = CScriptNum(pushes.back(), false).getint();
        pushes.pop_back();

        vector<unsigned char> vTx;
        if (!PopWithdrawPush(pushes, &vTx))
            return COutPoint();
        CTransaction tx;
        CDataStream(vTx, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_VERSION_MASK_BITCOIN_TX) >> tx;

        if (ntxOut < 0 || (unsigned int)ntxOut >= tx.vout.size())
            return COutPoint();

        return COutPoint(tx.GetBitcoinHash(), ntxOut);
    } catch (std::exception& e) {
        return COutPoint();
    }
}

void CScript::PushWithdraw(const vector<unsigned char> push) {
    int64_t pushCount = 0;
    for (vector<unsigned char>::const_iterator it = push.begin(); it < push.end(); pushCount++) {
        if (push.end() - it < 520) {
            *this << vector<unsigned char>(it, push.end());
            it = push.end();
        } else {
            *this << vector<unsigned char>(it, it + 520);
            it += 520;
        }
    }
    *this << pushCount;
}

bool CScript::IsPayToScriptHash() const
{
    // Extra-fast test for pay-to-script-hash CScripts:
    return (this->size() == 23 &&
            this->at(0) == OP_HASH160 &&
            this->at(1) == 0x14 &&
            this->at(22) == OP_EQUAL);
}

bool CScript::IsPushOnly() const
{
    const_iterator pc = begin();
    while (pc < end())
    {
        opcodetype opcode;
        if (!GetOp(pc, opcode))
            return false;
        // Note that IsPushOnly() *does* consider OP_RESERVED to be a
        // push-type opcode, however execution of OP_RESERVED fails, so
        // it's not relevant to P2SH/BIP62 as the scriptSig would fail prior to
        // the P2SH special validation code being executed.
        if (opcode > OP_16)
            return false;
    }
    return true;
}

std::string CScript::ToString() const
{
    std::string str;
    opcodetype opcode;
    std::vector<unsigned char> vch;
    const_iterator pc = begin();
    while (pc < end())
    {
        if (!str.empty())
            str += " ";
        if (!GetOp(pc, opcode, vch))
        {
            str += "[error]";
            return str;
        }
        if (0 <= opcode && opcode <= OP_PUSHDATA4)
            str += ValueString(vch);
        else
            str += GetOpName(opcode);
    }
    return str;
}
