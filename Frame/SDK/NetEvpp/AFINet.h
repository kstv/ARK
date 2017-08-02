// -------------------------------------------------------------------------
//    @FileName         ��    AFINet.h
//    @Author           ��    Ark Game Tech
//    @Date             ��    2013-12-15
//    @Module           ��    AFINet
//    @Desc             :     INet
// -------------------------------------------------------------------------

#ifndef AFI_NET_H
#define AFI_NET_H

#include <cstring>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <iostream>
#include <map>

#include "SDK/Core/AFGUID.h"

#ifndef _MSC_VER
#include <netinet/in.h>
#ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <vector>
#include <functional>
#include <memory>
#include <list>
#include <vector>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>
#include <event2/event_compat.h>
#include <assert.h>

#if NF_PLATFORM == NF_PLATFORM_WIN
//#ifdef _MSC_VER
#include <windows.h>
//#elseifdef _APPLE_
#elif NF_PLATFORM == NF_PLATFORM_APPLE
#include <libkern/OSByteOrder.h>
#else
#include <unistd.h>
#endif
#include "evpp/tcp_callbacks.h"
#include "evpp/buffer.h"

#pragma pack(push, 1)

enum NF_NET_EVENT
{
    NF_NET_EVENT_EOF = 0x10,        //����
    NF_NET_EVENT_ERROR = 0x20,      //δ֪����
    NF_NET_EVENT_TIMEOUT = 0x40,    //���ӳ�ʱ
    NF_NET_EVENT_CONNECTED = 0x80,  //���ӳɹ�(��Ϊ�ͻ���)
};


struct  AFIMsgHead
{
    enum NF_Head
    {
        NF_HEAD_LENGTH = 6,
    };

    virtual int EnCode(char* strData) = 0;
    virtual int DeCode(const char* strData) = 0;

    virtual uint16_t GetMsgID() const = 0;
    virtual void SetMsgID(uint16_t nMsgID) = 0;

    virtual uint32_t GetBodyLength() const = 0;
    virtual void SetBodyLength(uint32_t nLength) = 0;

    int64_t NF_HTONLL(int64_t nData)
    {
#if NF_PLATFORM == NF_PLATFORM_WIN
//#ifdef _MSC_VER
        return htonll(nData);
#elif NF_PLATFORM == NF_PLATFORM_APPLE
//#elseifdef __APPLE_CC__
        return OSSwapHostToBigInt64(nData);
#else
        return htobe64(nData);
#endif
    }

    int64_t NF_NTOHLL(int64_t nData)
    {
#if NF_PLATFORM == NF_PLATFORM_WIN
//#ifdef _MSC_VER
        return ntohll(nData);
#elif NF_PLATFORM == NF_PLATFORM_APPLE
//#elseifdef __APPLE__
        return OSSwapBigToHostInt64(nData);
#else
        return be64toh(nData);
#endif
    }

    int32_t NF_HTONL(int32_t nData)
    {
#if NF_PLATFORM == NF_PLATFORM_WIN
//#ifdef _MSC_VER
        return htonl(nData);
#elif NF_PLATFORM == NF_PLATFORM_APPLE
//#elseifdef __APPLE__
        return OSSwapHostToBigInt32(nData);
#else
        return htobe32(nData);
#endif
    }

    int32_t NF_NTOHL(int32_t nData)
    {
#if NF_PLATFORM == NF_PLATFORM_WIN
//#ifdef _MSC_VER
        return ntohl(nData);
#elif NF_PLATFORM == NF_PLATFORM_APPLE
//#elseifdef __APPLE__
        return OSSwapBigToHostInt32(nData);
#else
        return be32toh(nData);
#endif
    }

    int16_t NF_HTONS(int16_t nData)
    {
#if NF_PLATFORM == NF_PLATFORM_WIN
//#ifdef _MSC_VER
        return htons(nData);
#elif NF_PLATFORM == NF_PLATFORM_APPLE
//#elseifdef __APPLE__
        return OSSwapHostToBigInt16(nData);
#else
        return htobe16(nData);
#endif
    }

    int16_t NF_NTOHS(int16_t nData)
    {
#if NF_PLATFORM == NF_PLATFORM_WIN
//#ifdef _MSC_VER
        return ntohs(nData);
#elif NF_PLATFORM == NF_PLATFORM_APPLE
//#elseifdef __APPLE__
        return OSSwapBigToHostInt16(nData);
#else
        return be16toh(nData);
#endif
    }

};

class AFCMsgHead : public AFIMsgHead
{
public:
    AFCMsgHead()
    {
        munSize = 0;
        munMsgID = 0;
    }

    // Message Head[ MsgID(2) | MsgSize(4) ]
    virtual int EnCode(char* strData)
    {
        uint32_t nOffset = 0;

        uint16_t nMsgID = NF_HTONS(munMsgID);
        memcpy(strData + nOffset, (void*)(&nMsgID), sizeof(munMsgID));
        nOffset += sizeof(munMsgID);

        uint32_t nPackSize = munSize + NF_HEAD_LENGTH;
        uint32_t nSize = NF_HTONL(nPackSize);
        memcpy(strData + nOffset, (void*)(&nSize), sizeof(munSize));
        nOffset += sizeof(munSize);

        if(nOffset != NF_HEAD_LENGTH)
        {
            assert(0);
        }

        return nOffset;
    }

    // Message Head[ MsgID(2) | MsgSize(4) ]
    virtual int DeCode(const char* strData)
    {
        uint32_t nOffset = 0;

        uint16_t nMsgID = 0;
        memcpy(&nMsgID, strData + nOffset, sizeof(munMsgID));
        munMsgID = NF_NTOHS(nMsgID);
        nOffset += sizeof(munMsgID);

        uint32_t nPackSize = 0;
        memcpy(&nPackSize, strData + nOffset, sizeof(munSize));
        munSize = NF_NTOHL(nPackSize) - NF_HEAD_LENGTH;
        nOffset += sizeof(munSize);

        if(nOffset != NF_HEAD_LENGTH)
        {
            assert(0);
        }

        return nOffset;
    }

    virtual uint16_t GetMsgID() const
    {
        return munMsgID;
    }
    virtual void SetMsgID(uint16_t nMsgID)
    {
        munMsgID = nMsgID;
    }

    virtual uint32_t GetBodyLength() const
    {
        return munSize;
    }
    virtual void SetBodyLength(uint32_t nLength)
    {
        munSize = nLength;
    }
protected:
    uint32_t munSize;
    uint16_t munMsgID;
};
enum NetEventType
{
    None = 0,
    CONNECTED = 1,
    DISCONNECTED = 2,
    RECIVEDATA = 3,
};

class AFINet;

typedef std::function<void(const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& nClientID)> NET_RECEIVE_FUNCTOR;
typedef std::shared_ptr<NET_RECEIVE_FUNCTOR> NET_RECEIVE_FUNCTOR_PTR;

typedef std::function<void(const NetEventType nEvent, const AFGUID& nClientID, const int nServerID)> NET_EVENT_FUNCTOR;
typedef std::shared_ptr<NET_EVENT_FUNCTOR> NET_EVENT_FUNCTOR_PTR;

typedef std::function<void(int severity, const char* msg)> NET_EVENT_LOG_FUNCTOR;
typedef std::shared_ptr<NET_EVENT_LOG_FUNCTOR> NET_EVENT_LOG_FUNCTOR_PTR;


class NetObject
{
public:
    NetObject(AFINet* pNet, const AFGUID& xClientID, const evpp::TCPConnPtr& conn): mConnPtr(conn)
    {
        bNeedRemove = false;
        m_pNet = pNet;
        mnClientID = xClientID;
        bBuffChange = false;
        memset(&sin, 0, sizeof(sin));
    }

    virtual ~NetObject()
    {
    }

    int AddBuff(const char* str, uint32_t nLen)
    {
        mstrBuff.Write(str, nLen);
        bBuffChange = true;
        return (int)mstrBuff.length();
    }

    int RemoveBuff(uint32_t nLen)
    {
        if(nLen > mstrBuff.length())
        {
            return 0;
        }

        mstrBuff.Next(nLen);

        return mstrBuff.length();
    }

    const char* GetBuff()
    {
        return mstrBuff.data();
    }

    int GetBuffLen() const
    {
        return mstrBuff.length();
    }

    AFINet* GetNet()
    {
        return m_pNet;
    }
    bool NeedRemove()
    {
        return bNeedRemove;
    }
    void SetNeedRemove(bool b)
    {
        bNeedRemove = b;
    }
    const std::string& GetAccount() const
    {
        return mstrUserData;
    }

    void SetAccount(const std::string& strData)
    {
        mstrUserData = strData;
    }

    const AFGUID& GetClientID()
    {
        return mnClientID;
    }

    void SetClientID(const AFGUID& xClientID)
    {
        mnClientID = xClientID;
    }
    void Reset()
    {
        bBuffChange =  false;
    }
    bool BuffChange()
    {
        return bBuffChange;
    }
    const evpp::TCPConnPtr& GetConnPtr()
    {
        return mConnPtr;
    }

private:
    sockaddr_in sin;
    const evpp::TCPConnPtr mConnPtr;
    evpp::Buffer mstrBuff;
    std::string mstrUserData;
    AFGUID mnClientID;//temporary client id

    AFINet* m_pNet;
    bool bNeedRemove;
    bool bBuffChange;

};


struct MsgFromNetInfo
{
    MsgFromNetInfo(const evpp::TCPConnPtr TCPConPtr) : mTCPConPtr(TCPConPtr)
    {
        nType = None;
    }

    NetEventType nType;
    AFGUID xClientID;
    evpp::TCPConnPtr mTCPConPtr;
    std::string strMsg;
};

class AFINet
{
public:
    //need to call this function every frame to drive network library
    virtual bool Execute() = 0;

    virtual void Initialization(const std::string& strAddrPort, const int nServerID) {};
    virtual int Initialization(const unsigned int nMaxClient, const std::string& strAddrPort, const int nServerID, const int nCpuCount)
    {
        return -1;
    };

    virtual bool Final() = 0;

    //send a message with out msg-head[auto add msg-head in this function]
    virtual bool SendMsgWithOutHead(const int16_t nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID) = 0;

    //send a message to all client[need to add msg-head for this message by youself]
    virtual bool SendMsgToAllClient(const char* msg, const uint32_t nLen)
    {
        return false;
    }

    //send a message with out msg-head to all client[auto add msg-head in this function]
    virtual bool SendMsgToAllClientWithOutHead(const int16_t nMsgID, const char* msg, const uint32_t nLen)
    {
        return false;
    }

    virtual bool CloseNetObject(const AFGUID& xClientID) = 0;

    virtual bool IsServer() = 0;

    virtual bool Log(int severity, const char* msg) = 0;
    bool IsStop()
    {
        return  !bWorking;
    };
    virtual bool StopAfter(double dTime)
    {
        return false;
    };

protected:
    bool bWorking;

public:
    int nReceiverSize;
    int nSendSize;
};

#pragma pack(pop)

#endif
