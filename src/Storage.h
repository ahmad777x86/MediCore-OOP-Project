#pragma once
// ── Storage<T> ────────────────────────────────────────────────────────────────
// Generic container. Uses T* data[100] (array of pointers).
// Objects are dynamically allocated OUTSIDE and passed in.
// Storage does NOT own the objects (no delete in remove).
// Call clear() if you want ownership cleanup.
template <typename T>
class Storage
{
private:
    T *data[100]; // permitted static array per spec
    int count;

public:
    Storage() : count(0)
    {
        for (int i = 0; i < 100; i++)
            data[i] = nullptr;
    }

    // add a dynamically-allocated object
    bool add(T *item)
    {
        if (count >= 100)
            return false;
        data[count++] = item;
        return true;
    }

    // remove by ID (does NOT delete memory — caller decides)
    bool removeById(int id)
    {
        for (int i = 0; i < count; i++)
        {
            if (data[i] && data[i]->getId() == id)
            {
                for (int j = i; j < count - 1; j++)
                    data[j] = data[j + 1];
                data[--count] = nullptr;
                return true;
            }
        }
        return false;
    }

    // find by ID — returns pointer or nullptr
    T *findById(int id) const
    {
        for (int i = 0; i < count; i++)
            if (data[i] && data[i]->getId() == id)
                return data[i];
        return nullptr;
    }

    // returns raw pointer to internal array
    T **getAll() { return data; }

    int size() const { return count; }

    // delete all owned objects (call when storage owns memory)
    void clear()
    {
        for (int i = 0; i < count; i++)
        {
            delete data[i];
            data[i] = nullptr;
        }
        count = 0;
    }

    // get max ID in collection (for auto-increment)
    int maxId() const
    {
        int mx = 0;
        for (int i = 0; i < count; i++)
            if (data[i] && data[i]->getId() > mx)
                mx = data[i]->getId();
        return mx;
    }
};