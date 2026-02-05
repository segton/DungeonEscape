// Copyright Epic Games, Inc. All Rights Reserved.

#include "DungeeonEscapeCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DungeeonEscape.h"

#include	"CollectableItem.h"
#include	"Lock.h"
ADungeeonEscapeCharacter::ADungeeonEscapeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void ADungeeonEscapeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADungeeonEscapeCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADungeeonEscapeCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADungeeonEscapeCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADungeeonEscapeCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ADungeeonEscapeCharacter::LookInput);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ADungeeonEscapeCharacter::Interact);

	}
	else
	{
		UE_LOG(LogDungeeonEscape, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ADungeeonEscapeCharacter::Interact()
{
	FVector Start;
	FVector End;
	
	Start = FirstPersonCameraComponent->GetComponentLocation();
	End = Start + (FirstPersonCameraComponent->GetForwardVector() * MaxInteractionDistance);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f);

	FCollisionShape InteractionSphere = FCollisionShape::MakeSphere(InteractionSphereRadius);

	//DrawDebugSphere(GetWorld(), End, InteractionSphereRadius, 20,FColor::Green, false, 5.0f);

	FHitResult HitResult;
	bool HasHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel2, InteractionSphere);
	
	if (HasHit)
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor->ActorHasTag("CollectableItem"))
		{
			//hit actor is a collectable item

			ACollectableItem* CollectableItem = Cast<ACollectableItem>(HitActor);
			if (CollectableItem)
			{
				ItemList.Add(CollectableItem->ItemName);
				CollectableItem->Destroy();
			}
		}
		else if (HitActor->ActorHasTag("Lock"))
		{
			//hit actor is a lock
			ALock* Lock = Cast<ALock>(HitActor);
			if (Lock)
			{
				if (!(Lock->GetIsKeyPlaced()))
				{
					//lock is empty
					int32 ItemsRemoved = ItemList.RemoveSingle(Lock->KeyItemName);
					if (ItemsRemoved)
					{
						Lock->SetIsKeyplaced(true);
						UE_LOG(LogTemp, Display, TEXT("Placed %s in lock"), *Lock->KeyItemName);

					}
					else
					{
						UE_LOG(LogTemp, Display, TEXT("You don't have the %s to place in the lock"), *Lock->KeyItemName);
					}

				}
				else
				{
					//lock has key
					//Add the KeyItemName into the ItemList
					//Deactivate the lock
					ItemList.Add(Lock->KeyItemName);
					Lock->SetIsKeyplaced(false);

				}
			}

		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("No Actor Hit"));
	}

}


void ADungeeonEscapeCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ADungeeonEscapeCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ADungeeonEscapeCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ADungeeonEscapeCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ADungeeonEscapeCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ADungeeonEscapeCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}
