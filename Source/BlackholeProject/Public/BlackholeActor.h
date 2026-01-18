#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlackholeActor.generated.h"

// 전방 선언
class UStaticMeshComponent;
class USphereComponent;
class UMaterialParameterCollection;

UCLASS()
class BLACKHOLEPROJECT_API ABlackholeActor : public AActor
{
    GENERATED_BODY()

public:
    ABlackholeActor();

protected:
    virtual void Tick(float DeltaTime) override;

public:
    // 1. 눈에 보이는 검은 구체
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComp;

    // 2. 중력이 미치는 범위
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* GravitySphereComp;

    // 3. 설정 변수들
    UPROPERTY(EditAnywhere, Category = "Blackhole Settings")
    float PullPower = 1000000.0f;

    UPROPERTY(EditAnywhere, Category = "Blackhole Settings")
    float LensingStrength = 50.0f;

    // [추가] 셰이더 구멍 크기 미세 조절용
    UPROPERTY(EditAnywhere, Category = "Blackhole Settings")
    float HoleSizeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Blackhole Settings")
    bool bCompensateLensForScale = true;

    UPROPERTY(EditAnywhere, Category = "Blackhole Settings")
    float LensRadiusMultiplier = 1.1f;

    UPROPERTY(EditAnywhere, Category = "Blackhole Settings")
    float LensRadiusOffsetUV = 0.0f;

    // 4. 셰이더 연동 (MPC)
    UPROPERTY(EditAnywhere, Category = "Blackhole Settings")
    UMaterialParameterCollection* BlackholeMPC;
};