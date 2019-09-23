// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "assert.h"
#include "core.h"
#include "protocol.h"
#include "util.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

//
// Main network
//

unsigned int pnSeed[] =
{
    0x12345678
};

class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0x05;
        pchMessageStart[1] = 0x05;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0x05;
        nDefaultPort = 5530;
        nRPCPort = 5531;
        // vAlertPubKey = ParseHex("");
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 20);
        nSubsidyHalvingInterval = 100000;

        // Build the genesis block. Note that the output of the genesis coinbase cannot
        // be spent as it did not originally exist in the database.
        const char* pszTimestamp = "vcoin sn";
        CTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CBigNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 1 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("") << OP_CHECKSIG;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = 1431517588;
        genesis.nBits    = 0x1e0fffff;
        genesis.nNonce   = 1486592;
        hashGenesisBlock = genesis.GetHash();
        /*
        //// debug print
        while (hashGenesisBlock > bnProofOfWorkLimit.getuint256()){
            if (++genesis.nNonce==0) break;
            hashGenesisBlock = genesis.GetHash();
        }

        printf("mainnet: %s\n", hashGenesisBlock.ToString().c_str());
        printf("mainnet: %s\n", genesis.hashMerkleRoot.ToString().c_str());
        printf("mainnet: %x\n", bnProofOfWorkLimit.GetCompact());
        genesis.print();

        */
        
        assert(hashGenesisBlock == uint256("0x00000b7e804f0de87e7752550ff04d7686a4599509897feefd7f03904eb45633"));
        assert(genesis.hashMerkleRoot == uint256("0x1576ef41775095b26a8f8f2bb65b693ec12230608a425aa84ee462381cae00e6"));

        vSeeds.push_back(CDNSSeedData("minkiz", "minkiz.co"));

#if BOOST_VERSION >= 106000
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,70);                    // VCoin addresses start with 'V'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,30);                    // VCoin script addresses start with '7'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,96 + 128);              // VCoin private keys start with '7' or 'V'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >(); // Chaincoin BIP32 pubkeys start with 'drkv'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >(); // Chaincoin BIP32 prvkeys start with 'drkp'
#else
        base58Prefixes[PUBKEY_ADDRESS] = list_of(70);
        base58Prefixes[SCRIPT_ADDRESS] = list_of(30);
        base58Prefixes[SECRET_KEY]     = list_of(224);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xAD)(0xE4);
#endif
        // Convert the pnSeeds array into usable address objects.
        for (unsigned int i = 0; i < ARRAYLEN(pnSeed); i++)
        {
            // It'll only connect to one or two seed nodes because once it connects,
            // it'll get a pile of addresses with newer timestamps.
            // Seed nodes are given a random 'last seen time' of between one and two
            // weeks ago.
            const int64_t nOneWeek = 7*24*60*60;
            struct in_addr ip;
            memcpy(&ip, &pnSeed[i], sizeof(ip));
            CAddress addr(CService(ip, GetDefaultPort()));
            addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
            vFixedSeeds.push_back(addr);
        }
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet (v3)
//
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0x01;
        pchMessageStart[1] = 0xfe;
        pchMessageStart[2] = 0xfe;
        pchMessageStart[3] = 0x05;
        vAlertPubKey = ParseHex("");
        nDefaultPort = 55534;
        nRPCPort = 55535;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 20);
        strDataDir = "testnet3";

        // Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1374901774;
        genesis.nBits = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = 243324;

        //// debug print
        hashGenesisBlock = genesis.GetHash();
        /*
        //while (hashGenesisBlock > bnProofOfWorkLimit.getuint256()){
        //    if (++genesis.nNonce==0) break;
        //   hashGenesisBlock = genesis.GetHash();
        //}

        printf("testnet: %s\n", hashGenesisBlock.ToString().c_str());
        printf("testnet: %s\n", genesis.hashMerkleRoot.ToString().c_str());
        printf("testnet: %x\n", bnProofOfWorkLimit.GetCompact());
        genesis.print();

        */

        assert(hashGenesisBlock == uint256("0x0000031225834a1423f72fc7f8371e46b0ed172da9a9242edb891902abb85759"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("minkiz", "minkiz.co"));

#if BOOST_VERSION >= 106000
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,130);                    // VCoin addresses start with 'V'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,30);                    // VCoin script addresses start with '7'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,111 + 128);              // VCoin private keys start with '7' or 'V'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >(); // Chaincoin BIP32 pubkeys start with 'drkv'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >(); // Chaincoin BIP32 prvkeys start with 'drkp'
#else
        base58Prefixes[PUBKEY_ADDRESS] = list_of(130);
        base58Prefixes[SCRIPT_ADDRESS] = list_of(30);
        base58Prefixes[SECRET_KEY]     = list_of(239);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x35)(0x87)(0xCF);
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x35)(0x83)(0x94);
#endif
    }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;


//
// Regression test
//
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0x0f;
        pchMessageStart[2] = 0xa5;
        pchMessageStart[3] = 0x5a;
        nSubsidyHalvingInterval = 150;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 1);
        genesis.nTime = 1296688602;
        genesis.nBits = 0x207fffff;
        genesis.nNonce = 3;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 18444;
        strDataDir = "regtest";
        
        //// debug print
        hashGenesisBlock = genesis.GetHash();
        /*
        //while (hashGenesisBlock > bnProofOfWorkLimit.getuint256()){
        //    if (++genesis.nNonce==0) break;
        //    hashGenesisBlock = genesis.GetHash();
        //}

        printf("regtest: %s\n", hashGenesisBlock.ToString().c_str());
        printf("regtest: %s\n", genesis.hashMerkleRoot.ToString().c_str());
        printf("regtest: %x\n", bnProofOfWorkLimit.GetCompact());
        genesis.print();

        */

        // assert(hashGenesisBlock == uint256(""));

        vSeeds.clear();  // Regtest mode doesn't have any DNS seeds.

#if BOOST_VERSION >= 106000
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);                    // VCoin addresses start with 'V'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);                    // VCoin script addresses start with '7'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);              // VCoin private keys start with '7' or 'V'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >(); // Chaincoin BIP32 pubkeys start with 'drkv'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >(); // Chaincoin BIP32 prvkeys start with 'drkp'
#else
        base58Prefixes[PUBKEY_ADDRESS] = list_of(0);
        base58Prefixes[SCRIPT_ADDRESS] = list_of(5);
        base58Prefixes[SECRET_KEY]     = list_of(128);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x35)(0x87)(0xCF);
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x35)(0x83)(0x94);
#endif
    }

    virtual bool RequireRPCPassword() const { return false; }
    virtual Network NetworkID() const { return CChainParams::REGTEST; }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = &mainParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;
        case CChainParams::REGTEST:
            pCurrentParams = &regTestParams;
            break;
        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {
    bool fRegTest = GetBoolArg("-regtest", false);
    bool fTestNet = GetBoolArg("-testnet", false);

    if (fTestNet && fRegTest) {
        return false;
    }

    if (fRegTest) {
        SelectParams(CChainParams::REGTEST);
    } else if (fTestNet) {
        SelectParams(CChainParams::TESTNET);
    } else {
        SelectParams(CChainParams::MAIN);
    }
    return true;
}
