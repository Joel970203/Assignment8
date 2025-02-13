#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "SpartaPlayerController.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "HealingItem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

ASpartaGameState::ASpartaGameState()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	LevelDuration = 30.0f;
	CurrentLevelIndex = 0;
	MaxLevels = 3;

	// ���̺� ����
	CurrentWaveIndex = 0;
	MaxWaves = 3;
	WaveDurations = { 10.0f, 15.0f, 20.0f };  // ���̺꺰 ���� �ð�

	// ���̺꺰 ���� ���� ���� (��: 10, 20, 30���� ����)
	CoinsPerWave = { 10, 20, 30 };
}

void ASpartaGameState::BeginPlay()
{
	Super::BeginPlay();

	StartLevel();

	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ASpartaGameState::UpdateHUD,
		0.1f,
		true
	);
}


void ASpartaGameState::StartLevel()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->ShowGameHUD();
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
		}
	}

	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	StartWave();  // ù ��° ���̺� ����
}



void ASpartaGameState::StartWave()
{

	if (CurrentWaveIndex >= MaxWaves)
	{
		EndLevel();  // ��� ���̺갡 ������ ���� ����
		return;
	}

	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	UE_LOG(LogTemp, Warning, TEXT("Current Wave : %d "), CurrentWaveIndex + 1);  // ���̺� ��ȣ ���

	// ���̺갡 ���� �� ���� ���ΰ� ���� ������ ����
	if (CurrentWaveIndex > 0) // ù ��° ���̺� ����
	{
		// ���� ���� ����
		TArray<AActor*> FoundCoins;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACoinItem::StaticClass(), FoundCoins);
		for (AActor* CoinActor : FoundCoins)
		{
			if (CoinActor)
			{
				CoinActor->Destroy();
			}
		}

		// healing ������ ����
		TArray<AActor*> FoundHealingItems;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHealingItem::StaticClass(), FoundHealingItems);
		for (AActor* HealingActor : FoundHealingItems)
		{
			if (HealingActor)
			{
				HealingActor->Destroy();
			}
		}
	}

	// ���� ���� ��������
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

	if (FoundVolumes.Num() > 0)
	{
		ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
		if (SpawnVolume)
		{
			int32 ItemsToSpawn = CoinsPerWave[CurrentWaveIndex];

			// ù ��° ���̺꿡���� mine, healing, coin ��� ����
			if (CurrentWaveIndex == 0)
			{
				for (int32 i = 0; i < ItemsToSpawn; i++)
				{
					AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
					if (SpawnedActor)
					{
						if (SpawnedActor->IsA(ACoinItem::StaticClass()))
						{
							SpawnedCoinCount++;
						}
					}
				}
			}
			else // �� ���� ���̺꿡���� coin�� healing �����۸� �����ϰ� mine, coin�� ����
			{
				for (int32 i = 0; i < ItemsToSpawn; i++)
				{
					AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
					if (SpawnedActor)
					{
						if (SpawnedActor->IsA(ACoinItem::StaticClass()))
						{
							SpawnedCoinCount++;
						}
					}
				}
			}
		}
	}

	// ���̺� Ÿ�̸� ����
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&ASpartaGameState::OnWaveTimeUp,
		WaveDurations.IsValidIndex(CurrentWaveIndex) ? WaveDurations[CurrentWaveIndex] : 10.0f,  // �迭���� ���������� ����
		false
	);
}


void ASpartaGameState::OnWaveTimeUp()
{
	CurrentWaveIndex++;
	StartWave();  // ���� ���̺� ����
}

void ASpartaGameState::OnCoinCollected()
{
	CollectedCoinCount++;

	// ���� ���̺��� ��� ������ ��Ҵ��� Ȯ��
	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{
		GetWorldTimerManager().ClearTimer(WaveTimerHandle); // Ÿ�̸� ����

		CurrentWaveIndex++;

		if (CurrentWaveIndex < MaxWaves)
		{
			StartWave(); // ���� ���̺� ����
		}
		else
		{
			EndLevel(); // ������ ���̺���� �����ٸ� ���� ����
		}
	}
}



int32 ASpartaGameState::GetScore() const
{
	return Score;
}

void ASpartaGameState::AddScore(int32 Amount)
{
	// ���� ������ �߰�
	Score += Amount;

	// ���� �ν��Ͻ��� ���� �ݿ�
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			SpartaGameInstance->AddToScore(Amount);  // ���� �ݿ�
		}
	}

	// ���� ������Ʈ �α�
	UE_LOG(LogTemp, Warning, TEXT("Total Score Updated: %d"), Score);
}





void ASpartaGameState::OnLevelTimeUp()
{
	EndLevel();
}



void ASpartaGameState::EndLevel()
{
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			// ���� �ν��Ͻ����� ���� �ݿ�
			//SpartaGameInstance->AddToScore(Score);  // ���� �߰��� ���⼭�� �ݿ�

			CurrentLevelIndex++;
			SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;

			// ������ ���� ���� üũ
			int32 RequiredScore = 300; 
			UE_LOG(LogTemp, Warning, TEXT("RequiredScore: %d, Current Score: %d"), RequiredScore, Score);

			// ������ ���غ��� ������ ���� ���� ó��
			if (Score < RequiredScore)
			{
				UE_LOG(LogTemp, Warning, TEXT("Current Score: %d"), Score);
				OnGameOver();
				return;
			}

			// ���� ���� ����
			if (CurrentLevelIndex >= MaxLevels)
			{
				OnGameOver(); // ������ �������� �����ٸ� ���� ����
				return;
			}

			if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
			{
				UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
			}
			else
			{
				OnGameOver();
			}
		}
	}
}





void ASpartaGameState::OnGameOver()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->OnGameOver(); // Game Over ȭ���� ���
		}
	}
}

void ASpartaGameState::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController);
		{
			if (UUserWidget* HUDWidget = SpartaPlayerController->GetHUDWidget())
			{
				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
				{
					// ���̺� Ÿ�̸��� ���� �ð� ���
					float RemainingTime = GetWorldTimerManager().GetTimerRemaining(WaveTimerHandle);
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
				}

				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
						if (SpartaGameInstance)
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
						}
					}
				}

				if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1)));
				}

				if (UTextBlock* WaveIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Wave"))))
				{
					WaveIndexText->SetText(FText::FromString(FString::Printf(TEXT("Wave: %d/%d"), CurrentWaveIndex + 1, MaxWaves)));
				}
			}
		}
	}
}
