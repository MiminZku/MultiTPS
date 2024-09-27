// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(NetLog, Log, All);

#define LOCALROLE (UEnum::GetValueAsString<ENetRole>(GetLocalRole()))
#define REMOTEROLE (UEnum::GetValueAsString<ENetRole>(GetRemoteRole()))
#define NETMODE (GetNetMode() == ENetMode::NM_Client ? TEXT("Client") :\
				 GetNetMode() == ENetMode::NM_Standalone ? TEXT("Standalone") : TEXT("Server"))

#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))

#define PRINTLOG(fmt, ...) UE_LOG(NetLog, Warning, TEXT("[%s]%s : %s"), NETMODE, *CALLINFO, \
		*FString::Printf(fmt, ##__VA_ARGS__))