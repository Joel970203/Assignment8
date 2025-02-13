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
	ShowGameOverWidget(true); // ���� ���� ���� ǥ��

	// Ŭ���� �޽����� ���� ���� ǥ���ϴ� �Լ� ȣ��
	ShowGameClearMessage();

	SetPause(true); // ������ �Ͻ� ����

	bShowMouseCursor = true; // ���콺 Ŀ���� ǥ��

	SetInputMode(FInputModeUIOnly()); // UI�� ��ȣ�ۿ� �����ϵ��� ����
}


void ASpartaPlayerController::ShowGameClearMessage()
{
	// GameOverWidgetInstance�� null�̶�� ����
	if (!GameOverWidgetInstance)
	{
		GameOverWidgetInstance = GetGameOverWidgetInstance();
	}

	if (GameOverWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameOverWidgetInstance is valid"));

		// ClearText�� �ִ��� Ȯ���ϰ�, ����, ���̺�, ���� ���ǿ� ���� �ؽ�Ʈ ����
		if (UTextBlock* ClearTextBlock = Cast<UTextBlock>(GameOverWidgetInstance->GetWidgetFromName("GameOverText")))
		{
			UE_LOG(LogTemp, Warning, TEXT("ClearTextBlock found"));

			int32 CurrentScore = 0;
			if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				CurrentScore = SpartaGameInstance->TotalScore;
				UE_LOG(LogTemp, Warning, TEXT("Current Score: %d"), CurrentScore);
			}

			// ���� 3 �̻�, ���̺� 3 �̻�, ���� 900 �̻��� �� Ŭ���� �޽���
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

			// �� ���� �ؽ�Ʈ ����
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

		if (GameOverWidgetClass)  // GameOverWidgetClass�� ����� �ν��Ͻ� ����
		{
			GameOverWidgetInstance = CreateWidget<UUserWidget>(this, GameOverWidgetClass);  // ���� ���� ��� Ŭ���� ��� ���
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

		// Game Over ȭ���� �ƴ϶�� ���� HUD�� ǥ���ϵ��� ó��
		ShowGameHUD();
	}


}

UUserWidget* ASpartaPlayerController::GetGameOverWidgetInstance()
{
	if (!GameOverWidgetClass)
	{
		return nullptr; // GameOverWidgetClass�� null�� ��� ��ȯ
	}

	// GameOverWidgetInstance�� null�� ��츸 ���� ����
	if (!GameOverWidgetInstance)
	{
		GameOverWidgetInstance = CreateWidget<UUserWidget>(this, GameOverWidgetClass);  // Ŭ���� ��� ���
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