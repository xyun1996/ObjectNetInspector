#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "ObjectNetTypes.h"

/** Query used to filter object network events and aggregate rows. */
struct FObjectNetQuery
{
    FString SearchText;
    TOptional<uint32> ConnectionFilter;
    TOptional<double> TimeStartSec;
    TOptional<double> TimeEndSec;
    bool bIncludeRpc;
    bool bIncludeProperties;
    TOptional<EObjectNetDirection> DirectionFilter;

    FObjectNetQuery()
        : bIncludeRpc(true)
        , bIncludeProperties(true)
    {
    }

    bool PassesEvent(const FObjectNetEvent& Event) const
    {
        if (ConnectionFilter.IsSet() && Event.ConnectionId != ConnectionFilter.GetValue())
        {
            return false;
        }

        if (TimeStartSec.IsSet() && Event.TimeSec < TimeStartSec.GetValue())
        {
            return false;
        }

        if (TimeEndSec.IsSet() && Event.TimeSec > TimeEndSec.GetValue())
        {
            return false;
        }

        if (DirectionFilter.IsSet() && Event.Direction != DirectionFilter.GetValue())
        {
            return false;
        }

        if (Event.Kind == EObjectNetEventKind::Rpc && !bIncludeRpc)
        {
            return false;
        }

        if (Event.Kind == EObjectNetEventKind::Property && !bIncludeProperties)
        {
            return false;
        }

        if (!SearchText.IsEmpty())
        {
            const ESearchCase::Type SearchCase = ESearchCase::IgnoreCase;
            const bool bMatch =
                Event.ObjectName.Contains(SearchText, SearchCase) ||
                Event.ObjectPath.Contains(SearchText, SearchCase) ||
                Event.ClassName.Contains(SearchText, SearchCase) ||
                Event.EventName.Contains(SearchText, SearchCase);

            if (!bMatch)
            {
                return false;
            }
        }

        return true;
    }

    bool PassesAggregate(const FObjectNetAggregate& Aggregate) const
    {
        if (!SearchText.IsEmpty())
        {
            const ESearchCase::Type SearchCase = ESearchCase::IgnoreCase;
            const bool bTextMatch =
                Aggregate.ObjectName.Contains(SearchText, SearchCase) ||
                Aggregate.ObjectPath.Contains(SearchText, SearchCase) ||
                Aggregate.ClassName.Contains(SearchText, SearchCase);

            if (!bTextMatch)
            {
                return false;
            }
        }

        const bool bAnyTypeEnabled = bIncludeRpc || bIncludeProperties;
        if (!bAnyTypeEnabled)
        {
            return false;
        }

        if (bIncludeRpc && bIncludeProperties)
        {
            return Aggregate.TotalEventCount > 0;
        }

        if (bIncludeRpc)
        {
            return Aggregate.RpcCount > 0;
        }

        return Aggregate.PropertyCount > 0;
    }
};
