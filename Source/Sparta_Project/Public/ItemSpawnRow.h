#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" // FTableRowBase 정의가 들어있는 헤더
#include "ItemSpawnRow.generated.h"

USTRUCT(BlueprintType)
struct FItemSpawnRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 아이템 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemName;
    
    // 어떤 아이템 클래스를 스폰할지
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ItemClass; 
    
    // TSubclassOf 하드 레퍼런스 : 메모리에 로드된 상태에서 바로 접근 
    // TSoftClassPtr 소프트 레퍼런스  경로만 기억
    // 클래스를 참조하기 위한 데이터 구조들이다 !



    // 이 아이템의 스폰 확률
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnChance;
};