#pragma once

#include "CoreMinimal.h"
#include "FPS_Spring.generated.h"

USTRUCT(BlueprintType)
struct FFPS_Spring
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Spring")
	float Value = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Spring")
	float Velocity = 0.0f;

	static constexpr float MaxStep = 0.033f;

	FORCEINLINE void AddImpulse(float Impulse)
	{
		Velocity += Impulse;
	}

	FORCEINLINE void Tick(float Stiffness, float Damping, float DeltaTime)
	{
		const float Dt = FMath::Min(DeltaTime, MaxStep);

		Velocity += -Value * Stiffness * Dt;
		Velocity -= Velocity * Damping * Dt;
		Value += Velocity * Dt;
	}

	FORCEINLINE void Reset()
	{
		Value = 0.0f;
		Velocity = 0.0f;
	}
};

/** Vector variant. Used for viewmodel location, rotation as pitch/yaw/roll, and scale deltas. */
USTRUCT(BlueprintType)
struct FFPS_VectorSpring
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Spring")
	FVector Value = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Spring")
	FVector Velocity = FVector::ZeroVector;

	static constexpr float MaxStep = 0.033f;

	FORCEINLINE void AddImpulse(const FVector& Impulse)
	{
		Velocity += Impulse;
	}

	FORCEINLINE void Tick(float Stiffness, float Damping, float DeltaTime)
	{
		const float Dt = FMath::Min(DeltaTime, MaxStep);

		Velocity += -Value * Stiffness * Dt;
		Velocity -= Velocity * Damping * Dt;
		Value += Velocity * Dt;
	}

	FORCEINLINE FRotator ToRotator() const
	{
		return FRotator(Value.X, Value.Y, Value.Z);
	}

	FORCEINLINE void Reset()
	{
		Value = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
	}
};
