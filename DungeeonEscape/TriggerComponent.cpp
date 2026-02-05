// Fill out your copyright notice in the Description page of Project Settings.


#include "TriggerComponent.h"
UTriggerComponent::UTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	UE_LOG(LogTemp, Display, TEXT("TriggerComponent created"));
}
void UTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (MoverActor) //MoverActor != nullptr
	{
		Mover = MoverActor->FindComponentByClass<UMover>();

		if (Mover) //Mover != nullptr
		{
			UE_LOG(LogTemp, Display, TEXT("Successfully found Mover component"));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Failed to find Mover component"));

		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("MoverActor is nullptr"));

	}
	UE_LOG(LogTemp, Display, TEXT("Triggered"));

	if (IsPressureplate)
	{
		OnComponentBeginOverlap.AddDynamic(this, &UTriggerComponent::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &UTriggerComponent::OnOverlapEnd);
	}

}

void UTriggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UTriggerComponent::Trigger(bool NewTriggerValue)
{
		IsTriggered = NewTriggerValue;
		if(Mover)
		{
			Mover->SetShouldMove(IsTriggered);
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("%s doesn't have a mover to trigger"), *GetOwner()->GetActorNameOrLabel());

		}
}

void UTriggerComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->ActorHasTag("PressurePlateActivator"))
	{
		ActivatorCount++;
		if (!IsTriggered)
		{
			Trigger(true);

		}
	}
	

}

void UTriggerComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	if (OtherActor && OtherActor->ActorHasTag("PressurePlateActivator"))
	{
		ActivatorCount--;
		if (IsTriggered && !ActivatorCount)
		{
			Trigger(false);
		}
	}
}
