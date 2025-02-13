#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" // FTableRowBase ���ǰ� ����ִ� ���
#include "ItemSpawnRow.generated.h"

USTRUCT(BlueprintType)
struct FItemSpawnRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    // ������ �̸�
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemName;
    
    // � ������ Ŭ������ ��������
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ItemClass; 
    
    // TSubclassOf �ϵ� ���۷��� : �޸𸮿� �ε�� ���¿��� �ٷ� ���� 
    // TSoftClassPtr ����Ʈ ���۷���  ��θ� ���
    // Ŭ������ �����ϱ� ���� ������ �������̴� !



    // �� �������� ���� Ȯ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnChance;
};