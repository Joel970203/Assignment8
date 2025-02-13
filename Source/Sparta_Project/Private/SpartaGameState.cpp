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

	// 웨이브 설정
	CurrentWaveIndex = 0;
	MaxWaves = 3;
	WaveDurations = { 10.0f, 15.0f, 20.0f };  // 웨이브별 지속 시간

	// 웨이브별 스폰 개수 설정 (예: 10, 20, 30개씩 증가)
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

	StartWave();  // 첫 번째 웨이브 시작
}



void ASpartaGameState::StartWave()
{

	if (CurrentWaveIndex >= MaxWaves)
	{
		EndLevel();  // 모든 웨이브가 끝나면 레벨 종료
		return;
	}

	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	UE_LOG(LogTemp, Warning, TEXT("Current Wave : %d "), CurrentWaveIndex + 1);  // 웨이브 번호 출력

	// 웨이브가 끝난 후 이전 코인과 힐링 아이템 삭제
	if (CurrentWaveIndex > 0) // 첫 번째 웨이브 이후
	{
		// 기존 코인 제거
		TArray<AActor*> FoundCoins;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACoinItem::StaticClass(), FoundCoins);
		for (AActor* CoinActor : FoundCoins)
		{
			if (CoinActor)
			{
				CoinActor->Destroy();
			}
		}

		// healing 아이템 제거
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

	// 스폰 볼륨 가져오기
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

	if (FoundVolumes.Num() > 0)
	{
		ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
		if (SpawnVolume)
		{
			int32 ItemsToSpawn = CoinsPerWave[CurrentWaveIndex];

			// 첫 번째 웨이브에서는 mine, healing, coin 모두 스폰
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
			else // 그 이후 웨이브에서는 coin과 healing 아이템만 제거하고 mine, coin만 스폰
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

	// 웨이브 타이머 시작
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&ASpartaGameState::OnWaveTimeUp,
		WaveDurations.IsValidIndex(CurrentWaveIndex) ? WaveDurations[CurrentWaveIndex] : 10.0f,  // 배열에서 가져오도록 수정
		false
	);
}


void ASpartaGameState::OnWaveTimeUp()
{
	CurrentWaveIndex++;
	StartWave();  // 다음 웨이브 시작
}

void ASpartaGameState::OnCoinCollected()
{
	CollectedCoinCount++;

	// 현재 웨이브의 모든 코인을 모았는지 확인
	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{
		GetWorldTimerManager().ClearTimer(WaveTimerHandle); // 타이머 제거

		CurrentWaveIndex++;

		if (CurrentWaveIndex < MaxWaves)
		{
			StartWave(); // 다음 웨이브 시작
		}
		else
		{
			EndLevel(); // 마지막 웨이브까지 끝났다면 레벨 종료
		}
	}
}



int32 ASpartaGameState::GetScore() const
{
	return Score;
}

void ASpartaGameState::AddScore(int32 Amount)
{
	// 현재 점수에 추가
	Score += Amount;

	// 게임 인스턴스에 점수 반영
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			SpartaGameInstance->AddToScore(Amount);  // 점수 반영
		}
	}

	// 점수 업데이트 로그
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
			// 게임 인스턴스에서 점수 반영
			//SpartaGameInstance->AddToScore(Score);  // 점수 추가는 여기서만 반영

			CurrentLevelIndex++;
			SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;

			// 레벨별 점수 기준 체크
			int32 RequiredScore = 300; 
			UE_LOG(LogTemp, Warning, TEXT("RequiredScore: %d, Current Score: %d"), RequiredScore, Score);

			// 점수가 기준보다 적으면 게임 오버 처리
			if (Score < RequiredScore)
			{
				UE_LOG(LogTemp, Warning, TEXT("Current Score: %d"), Score);
				OnGameOver();
				return;
			}

			// 다음 레벨 진행
			if (CurrentLevelIndex >= MaxLevels)
			{
				OnGameOver(); // 마지막 레벨까지 끝났다면 게임 종료
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
			SpartaPlayerController->OnGameOver(); // Game Over 화면을 띄움
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
					// 웨이브 타이머의 남은 시간 계산
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
