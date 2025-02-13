#include "SpartaPlayerController.h"
#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

ASpartaPlayerController::ASpartaPlayerController()
	: InputMappingContext(nullptr),
	MoveAction(nullptr),
	JumpAction(nullptr),
	LookAction(nullptr),
	SprintAction(nullptr),
	HUDWidgetClass(nullptr),
	HUDWidgetInstance(nullptr),
	MainMenuWidgetClass(nullptr),
	MainMenuWidgetInstance(nullptr)
{
}

void ASpartaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("Menu_Level"))
	{
		ShowMainMenu(false);
	}
}

UUserWidget* ASpartaPlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

void ASpartaPlayerController::ShowMainMenu(bool bIsRestart)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();

			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}

		if (UTextBlock* ButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
		{
			if (bIsRestart)
			{
				ButtonText->SetText(FText::FromString(TEXT("Restart")));
			}
			else
			{
				ButtonText->SetText(FText::FromString(TEXT("Start")));
			}
		}

		if (bIsRestart)
		{
			UFunction* PlayAnimFunc = MainMenuWidgetInstance->FindFunction(FName("PlayGameOverAnim"));
			if (PlayAnimFunc)
			{
				MainMenuWidgetInstance->ProcessEvent(PlayAnimFunc, nullptr);
			}

			if (UTextBlock* TotalScoreText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName("TotalScoreText")))
			{
				if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
				{
					TotalScoreText->SetText(FText::FromString(
						FString::Printf(TEXT("Total Score: %d"), SpartaGameInstance->TotalScore)
					));
				}
			}
		}
	}
}


void ASpartaPlayerController::OnGameOver()
{
	ShowGameOverWidget(true); // 게임 오버 위젯 표시

	// 클리어 메시지와 점수 등을 표시하는 함수 호출
	ShowGameClearMessage();

	SetPause(true); // 게임을 일시 정지

	bShowMouseCursor = true; // 마우스 커서를 표시

	SetInputMode(FInputModeUIOnly()); // UI만 상호작용 가능하도록 설정
}


void ASpartaPlayerController::ShowGameClearMessage()
{
	// GameOverWidgetInstance가 null이라면 생성
	if (!GameOverWidgetInstance)
	{
		GameOverWidgetInstance = GetGameOverWidgetInstance();
	}

	if (GameOverWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameOverWidgetInstance is valid"));

		// ClearText가 있는지 확인하고, 레벨, 웨이브, 점수 조건에 맞춰 텍스트 설정
		if (UTextBlock* ClearTextBlock = Cast<UTextBlock>(GameOverWidgetInstance->GetWidgetFromName("GameOverText")))
		{
			UE_LOG(LogTemp, Warning, TEXT("ClearTextBlock found"));

			int32 CurrentScore = 0;
			if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				CurrentScore = SpartaGameInstance->TotalScore;
				UE_LOG(LogTemp, Warning, TEXT("Current Score: %d"), CurrentScore);
			}

			// 레벨 3 이상, 웨이브 3 이상, 점수 900 이상일 때 클리어 메시지
			if (CurrentScore >= 900)
			{
				ClearTextBlock->SetText(FText::FromString("You cleared the game!!"));
				UE_LOG(LogTemp, Warning, TEXT("You cleared the game!!"));
			}
			else
			{
				ClearTextBlock->SetText(FText::FromString("You didn't clear the game!!"));
				UE_LOG(LogTemp, Warning, TEXT("You didn't clear the game!!"));
			}

			// 총 점수 텍스트 설정
			if (UTextBlock* TotalScoreText = Cast<UTextBlock>(GameOverWidgetInstance->GetWidgetFromName("TotalScoreText")))
			{
				TotalScoreText->SetText(FText::FromString(FString::Printf(TEXT("Total Score: %d"), CurrentScore)));
				UE_LOG(LogTemp, Warning, TEXT("Total Score: %d"), CurrentScore);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ClearTextBlock not found!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameOverWidgetInstance is null!"));
	}
}



void ASpartaPlayerController::ShowGameOverWidget(bool bIsGameOver)
{
	if (bIsGameOver)
	{
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->RemoveFromParent();
			HUDWidgetInstance = nullptr;
		}

		if (GameOverWidgetClass)  // GameOverWidgetClass를 사용해 인스턴스 생성
		{
			GameOverWidgetInstance = CreateWidget<UUserWidget>(this, GameOverWidgetClass);  // 로컬 변수 대신 클래스 멤버 사용
			if (GameOverWidgetInstance)
			{
				GameOverWidgetInstance->AddToViewport();
				bShowMouseCursor = true;
				SetInputMode(FInputModeUIOnly());
			}
		}
	}
	else
	{
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->RemoveFromParent();
			HUDWidgetInstance = nullptr;
		}

		// Game Over 화면이 아니라면 기존 HUD를 표시하도록 처리
		ShowGameHUD();
	}


}

UUserWidget* ASpartaPlayerController::GetGameOverWidgetInstance()
{
	if (!GameOverWidgetClass)
	{
		return nullptr; // GameOverWidgetClass가 null인 경우 반환
	}

	// GameOverWidgetInstance가 null인 경우만 새로 생성
	if (!GameOverWidgetInstance)
	{
		GameOverWidgetInstance = CreateWidget<UUserWidget>(this, GameOverWidgetClass);  // 클래스 멤버 사용
	}

	return GameOverWidgetInstance;
}


void ASpartaPlayerController::ShowGameHUD()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();

			bShowMouseCursor = false;
			SetInputMode(FInputModeGameOnly());
		}

		ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
		if (SpartaGameState)
		{
			SpartaGameState->UpdateHUD();
		}
	}
}

void ASpartaPlayerController::StartGame()
{
	if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		SpartaGameInstance->CurrentLevelIndex = 0;
		SpartaGameInstance->TotalScore = 0;
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("BasicLevel"));
	SetPause(false);
}