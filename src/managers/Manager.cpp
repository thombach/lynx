#pragma once

class Manager
{
public:
    virtual ~Manager() = default;

    void setDirty(bool value = true)
    {
        dirty = value;
    }

    bool isDirty() const
    {
        return dirty;
    }

protected:
    bool dirty = true;
};