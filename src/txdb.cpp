// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"

#include "core.h"
#include "net.h"
#include "uint256.h"

#include <stdint.h>

using namespace std;

void static BatchWriteCoins(CLevelDBBatch &batch, const uint256 &hash, const CCoins &coins) {
    if (coins.IsPruned())
        batch.Erase(make_pair('c', hash));
    else
        batch.Write(make_pair('c', hash), coins);
}

void static BatchWriteHashBestChain(CLevelDBBatch &batch, const uint256 &hash) {
    batch.Write('B', hash);
}

CCoinsViewDB::CCoinsViewDB(size_t nCacheSize, bool fMemory, bool fWipe) : db(GetDataDir() / "chainstate", nCacheSize, fMemory, fWipe) {
}

bool CCoinsViewDB::GetCoins(const uint256 &txid, CCoins &coins) {
    return db.Read(make_pair('c', txid), coins);
}

bool CCoinsViewDB::SetCoins(const uint256 &txid, const CCoins &coins) {
    CLevelDBBatch batch;
    BatchWriteCoins(batch, txid, coins);
    return db.WriteBatch(batch);
}

bool CCoinsViewDB::HaveCoins(const uint256 &txid) {
    return db.Exists(make_pair('c', txid));
}

uint256 CCoinsViewDB::GetBestBlock() {
    uint256 hashBestChain;
    if (!db.Read('B', hashBestChain))
        return uint256(0);
    return hashBestChain;
}

bool CCoinsViewDB::SetBestBlock(const uint256 &hashBlock) {
    CLevelDBBatch batch;
    BatchWriteHashBestChain(batch, hashBlock);
    return db.WriteBatch(batch);
}

bool CCoinsViewDB::BatchWrite(const std::map<uint256, CCoins> &mapCoins, const uint256 &hashBlock) {
    LogPrint("coindb", "Committing %u changed transactions to coin database...\n", (unsigned int)mapCoins.size());

    CLevelDBBatch batch;
    for (std::map<uint256, CCoins>::const_iterator it = mapCoins.begin(); it != mapCoins.end(); it++)
        BatchWriteCoins(batch, it->first, it->second);
    if (hashBlock != uint256(0))
        BatchWriteHashBestChain(batch, hashBlock);

    return db.WriteBatch(batch);
}

CBlockTreeDB::CBlockTreeDB(size_t nCacheSize, bool fMemory, bool fWipe) : CLevelDBWrapper(GetDataDir() / "blocks" / "index", nCacheSize, fMemory, fWipe) {
}

bool CBlockTreeDB::WriteBlockIndex(const CDiskBlockIndex& blockindex)
{
    return Write(make_pair('b', blockindex.GetBlockHash()), blockindex);
}

bool CBlockTreeDB::WriteBestInvalidWork(const CBigNum& bnBestInvalidWork)
{
    // Obsolete; only written for backward compatibility.
    return Write('I', bnBestInvalidWork);
}

bool CBlockTreeDB::WriteBlockFileInfo(int nFile, const CBlockFileInfo &info) {
    return Write(make_pair('f', nFile), info);
}

bool CBlockTreeDB::ReadBlockFileInfo(int nFile, CBlockFileInfo &info) {
    return Read(make_pair('f', nFile), info);
}

bool CBlockTreeDB::WriteLastBlockFile(int nFile) {
    return Write('l', nFile);
}

bool CBlockTreeDB::WriteReindexing(bool fReindexing) {
    if (fReindexing)
        return Write('R', '1');
    else
        return Erase('R');
}

bool CBlockTreeDB::ReadReindexing(bool &fReindexing) {
    fReindexing = Exists('R');
    return true;
}

bool CBlockTreeDB::ReadLastBlockFile(int &nFile) {
    return Read('l', nFile);
}

bool CCoinsViewDB::GetStats(CCoinsStats &stats) {
    leveldb::Iterator *pcursor = db.NewIterator();
    pcursor->SeekToFirst();

    CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
    stats.hashBlock = GetBestBlock();
    ss << stats.hashBlock;
    int64_t nTotalAmount = 0;
    while (pcursor->Valid()) {
        boost::this_thread::interruption_point();
        try {
            leveldb::Slice slKey = pcursor->key();
            CDataStream ssKey(slKey.data(), slKey.data()+slKey.size(), SER_DISK, CLIENT_VERSION);
            char chType;
            ssKey >> chType;
            if (chType == 'c') {
                leveldb::Slice slValue = pcursor->value();
                CDataStream ssValue(slValue.data(), slValue.data()+slValue.size(), SER_DISK, CLIENT_VERSION);
                CCoins coins;
                ssValue >> coins;
                uint256 txhash;
                ssKey >> txhash;
                ss << txhash;
                ss << VARINT(coins.nVersion);
                ss << (coins.fCoinBase ? 'c' : 'n');
                ss << VARINT(coins.nHeight);
                stats.nTransactions++;
                for (unsigned int i=0; i<coins.vout.size(); i++) {
                    const CTxOut &out = coins.vout[i];
                    if (!out.IsNull()) {
                        stats.nTransactionOutputs++;
                        ss << VARINT(i+1);
                        ss << out;
                        nTotalAmount += out.nValue;
                    }
                }
                stats.nSerializedSize += 32 + slValue.size();
                ss << VARINT(0);
            }
            pcursor->Next();
        } catch (std::exception &e) {
            return error("%s : Deserialize or I/O error - %s", __func__, e.what());
        }
    }
    delete pcursor;
    stats.nHeight = mapBlockIndex.find(GetBestBlock())->second->nHeight;
    stats.hashSerialized = ss.GetHash();
    stats.nTotalAmount = nTotalAmount;
    return true;
}

bool CBlockTreeDB::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos) {
    return Read(make_pair('t', txid), pos);
}

bool CBlockTreeDB::WriteTxIndex(const std::vector<std::pair<uint256, CDiskTxPos> >&vect) {
    CLevelDBBatch batch;
    for (std::vector<std::pair<uint256,CDiskTxPos> >::const_iterator it=vect.begin(); it!=vect.end(); it++)
        batch.Write(make_pair('t', it->first), it->second);
    return WriteBatch(batch);
}

bool CBlockTreeDB::WriteFlag(const std::string &name, bool fValue) {
    return Write(std::make_pair('F', name), fValue ? '1' : '0');
}

bool CBlockTreeDB::ReadFlag(const std::string &name, bool &fValue) {
    char ch;
    if (!Read(std::make_pair('F', name), ch))
        return false;
    fValue = ch == '1';
    return true;
}

bool CBlockTreeDB::LoadBlockIndexGuts()
{
    leveldb::Iterator *pcursor = NewIterator();

    CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
    ssKeySet << make_pair('b', uint256(0));
    pcursor->Seek(ssKeySet.str());

    // Load mapBlockIndex
    while (pcursor->Valid()) {
        boost::this_thread::interruption_point();
        try {
            leveldb::Slice slKey = pcursor->key();
            CDataStream ssKey(slKey.data(), slKey.data()+slKey.size(), SER_DISK, CLIENT_VERSION);
            char chType;
            ssKey >> chType;
            if (chType == 'b') {
                leveldb::Slice slValue = pcursor->value();
                CDataStream ssValue(slValue.data(), slValue.data()+slValue.size(), SER_DISK, CLIENT_VERSION);
                CDiskBlockIndex diskindex;
                ssValue >> diskindex;

                // Construct block index object
                CBlockIndex* pindexNew = InsertBlockIndex(diskindex.GetBlockHash());
                pindexNew->pprev          = InsertBlockIndex(diskindex.hashPrev);
                pindexNew->nHeight        = diskindex.nHeight;
                pindexNew->nFile          = diskindex.nFile;
                pindexNew->nDataPos       = diskindex.nDataPos;
                pindexNew->nUndoPos       = diskindex.nUndoPos;
                pindexNew->nVersion       = diskindex.nVersion;
                pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
                pindexNew->nTime          = diskindex.nTime;
                pindexNew->nBits          = diskindex.nBits;
                pindexNew->nNonce         = diskindex.nNonce;
                pindexNew->nStatus        = diskindex.nStatus;
                pindexNew->nTx            = diskindex.nTx;

                if (!pindexNew->CheckIndex())
                    return error("LoadBlockIndex() : CheckIndex failed: %s", pindexNew->ToString());

                pcursor->Next();
            } else {
                break; // if shutdown requested or finished loading block index
            }
        } catch (std::exception &e) {
            return error("%s : Deserialize or I/O error - %s", __func__, e.what());
        }
    }
    delete pcursor;

    return true;
}

CTxOut getPrevOut(const COutPoint &out)
{
    CTransaction tx;
    uint256 hashBlock = 0;
    if (GetTransaction(out.hash, tx, hashBlock, true))
        return tx.vout[out.n];
    return CTxOut();
}

void getNextIn(const COutPoint &Out, uint256& Hash, unsigned int& n)
{
    Hash = 0;
    n = 0;
    if (paddressmap)
        paddressmap->ReadNextIn(Out, Hash, n);
}

// Return transaction in tx, and if it was found inside a block, its header is placed in block
bool ReadTransaction(const CDiskTxPos &postx, CTransaction &txOut, CBlockHeader &block)
{
    CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
    try
    {
        file >> block;
        fseek(file, postx.nTxOffset, SEEK_CUR);
        file >> txOut;
    }
    catch (std::exception &e)
    {
        return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
    }
    return true;
}

CAddressDB::CAddressDB(size_t nCacheSize, bool fMemory, bool fWipe) : CLevelDBWrapper(GetDataDir() / "blocks" / "addresses", nCacheSize, fMemory, fWipe)
{
}

bool CAddressDB::AddTx(const std::vector<CTransaction>& vtx, const std::vector<std::pair<uint256, CDiskTxPos> >& vpos)
{
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
        const CDiskTxPos& pos = vpos[i].second;
        uint256 TxHash = vtx[i].GetHash();

        std::vector<CScriptID> Inputs;
        for (unsigned int j = 0; j < vtx[i].vin.size(); j++)
        {
            const CTxIn& in = vtx[i].vin[j];
            CScript script = getPrevOut(in.prevout).scriptPubKey;
            if (script.empty())
                continue;
            CScriptID scid = script.GetID();

            // ignore inputs from the same address
            if (std::find(Inputs.begin(), Inputs.end(), scid) != Inputs.end())
                continue;
            Inputs.push_back(scid);

            std::vector<CDiskTxPos> Txs;
            Read(scid, Txs);
            Txs.push_back(pos);
            if (!Write(scid, Txs))
                return false;

            // store 'redeemed in' information for each tx output
            std::vector<std::pair<uint256, unsigned int> > Ins;
            Read(in.prevout.hash, Ins);
            if (in.prevout.n >= Ins.size())
                Ins.resize(in.prevout.n + 1);
            Ins[in.prevout.n] = std::pair<uint256, unsigned int>(TxHash, j);
            Write(in.prevout.hash, Ins);
        }
        BOOST_FOREACH (const CTxOut& out, vtx[i].vout)
        {
            CScriptID scid = out.scriptPubKey.GetID();
            std::vector<CDiskTxPos> Txs;
            Read(scid, Txs);
            Txs.push_back(pos);
            if (!Write(scid, Txs))
                return false;
        }
    }
    return true;
}

bool CAddressDB::GetTxs(std::vector<CDiskTxPos>& Txs, const CScriptID &Address)
{
    return Read(Address, Txs);
}
bool CAddressDB::ReadNextIn(const COutPoint &Out, uint256& Hash, unsigned int& n)
{
    std::vector<std::pair<uint256, unsigned int> > Ins;
    if (!Read(Out.hash, Ins) || Out.n >= Ins.size())
        return false;
    Hash = Ins[Out.n].first;
    n = Ins[Out.n].second;
    return true;
}

bool CAddressDB::WriteReindexing(bool fReindexing) {
    if (fReindexing)
        return Write('R', '1');
    else
        return Erase('R');
}

bool CAddressDB::ReadReindexing(bool &fReindexing) {
    fReindexing = Exists('R');
    return true;
}

bool CAddressDB::WriteEnable( bool fValue) {
    return Write(std::string("Faddrindex"), fValue ? '1' : '0');
}

bool CAddressDB::ReadEnable( bool &fValue) {
    char ch;
    if (!Read(std::string("Faddrindex"), ch))
        return false;
    fValue = ch == '1';
    return true;
}
