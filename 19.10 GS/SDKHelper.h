#pragma once
#include <cstring>
#include <cstdint>

namespace SDK
{
    inline bool operator==(const FUniqueNetIdRepl& lhs, const FUniqueNetIdRepl& rhs)
    {
        if (lhs.ReplicationBytes.Num() != rhs.ReplicationBytes.Num())
            return false;

        if (lhs.ReplicationBytes.Num() == 0)
            return true;

        if (!lhs.ReplicationBytes.Data || !rhs.ReplicationBytes.Data)
            return lhs.ReplicationBytes.Data == rhs.ReplicationBytes.Data;

        return memcmp(lhs.ReplicationBytes.Data, rhs.ReplicationBytes.Data, lhs.ReplicationBytes.Num()) == 0;
    }

    inline bool operator!=(const FUniqueNetIdRepl& lhs, const FUniqueNetIdRepl& rhs)
    {
        return !(lhs == rhs);
    }
}
