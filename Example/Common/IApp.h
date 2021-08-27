#pragma once


class IApp
{
    public:
    virtual bool init() { return false;}
    virtual void run() {}
    virtual void mainLoop(){}
    virtual void destory() {}
};