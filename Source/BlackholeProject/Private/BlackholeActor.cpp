#include "BlackholeActor.h"

// [필수 헤더]
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

ABlackholeActor::ABlackholeActor()
{
    PrimaryActorTick.bCanEverTick = true; // 심장 박동 켜기

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    GravitySphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("GravitySphereComp"));
    GravitySphereComp->SetupAttachment(MeshComp);
}

void ABlackholeActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 셰이더에 위치 전송
    if (BlackholeMPC)
    {
        UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(BlackholeMPC);
        APlayerController* PC = GetWorld()->GetFirstPlayerController();

        if (MPCInstance && PC)
        {
            const FVector ActorLocation = GetActorLocation();
            FVector2D ScreenPos;
            int32 SizeX, SizeY;
            PC->GetViewportSize(SizeX, SizeY);

            // 1. 3D 월드 좌표 -> 2D 화면 좌표 변환
            // (내 몸통 위치가 화면 어디쯤이니?)
            bool bIsOnScreen = PC->ProjectWorldLocationToScreen(ActorLocation, ScreenPos, false);

            if (SizeX > 0 && SizeY > 0)
            {
                // 2. UV 좌표로 변환 (0~1 사이 값)
                float UV_X = ScreenPos.X / (float)SizeX;
                float UV_Y = ScreenPos.Y / (float)SizeY;

                // 3. 카메라 뒤쪽 체크 (뒤에 있으면 렌즈 끄기)
                // (이걸 안 하면 뒤돌았을 때 왜곡이 반대로 튀어나옵니다)
                FVector CamLoc;
                FRotator CamRot;
                PC->GetPlayerViewPoint(CamLoc, CamRot);
                FVector DirToTarget = ActorLocation - CamLoc;
                float DotProd = FVector::DotProduct(CamRot.Vector(), DirToTarget);
                const bool bIsBehindCamera = DotProd < 0.0f;

                if (bIsBehindCamera) // 카메라 뒤에 있다면?
                {
                    UV_X = -10.0f; // 화면 밖으로 던져버림
                }

                float ScreenRadiusUV = 0.0f;
                if (bIsOnScreen && !bIsBehindCamera && MeshComp)
                {
                    const float WorldRadius = MeshComp->Bounds.SphereRadius;
                    const FVector CamRight = CamRot.Quaternion().GetRightVector();
                    const FVector CamUp = CamRot.Quaternion().GetUpVector();
                    FVector2D ScreenPosRight;
                    FVector2D ScreenPosUp;

                    const bool bRightOnScreen = PC->ProjectWorldLocationToScreen(ActorLocation + CamRight * WorldRadius, ScreenPosRight, false);
                    const bool bUpOnScreen = PC->ProjectWorldLocationToScreen(ActorLocation + CamUp * WorldRadius, ScreenPosUp, false);

                    if (bRightOnScreen && bUpOnScreen)
                    {
                        const float RadiusX = FMath::Abs(ScreenPosRight.X - ScreenPos.X) / (float)SizeX;
                        const float RadiusY = FMath::Abs(ScreenPosUp.Y - ScreenPos.Y) / (float)SizeY;
                        ScreenRadiusUV = FMath::Max(RadiusX, RadiusY);
                    }
                }

                float EffectiveHoleSizeMultiplier = HoleSizeMultiplier;
                if (bCompensateLensForScale && MeshComp)
                {
                    const float Scale = MeshComp->GetComponentScale().GetAbsMax();
                    EffectiveHoleSizeMultiplier *= (1.0f / FMath::Max(Scale, KINDA_SMALL_NUMBER));
                }

                const float HoleRadiusUV = ScreenRadiusUV * EffectiveHoleSizeMultiplier;
                const float LensRadiusUV = HoleRadiusUV * LensRadiusMultiplier + LensRadiusOffsetUV;

                // 4. 데이터 전송 (X, Y 좌표)
                // Z, W는 안 쓰면 0으로 채움
                MPCInstance->SetVectorParameterValue(TEXT("BlackholePos"), FLinearColor(UV_X, UV_Y, 0, 0));
                MPCInstance->SetScalarParameterValue(TEXT("LensingStrength"), LensingStrength);
                MPCInstance->SetScalarParameterValue(TEXT("HoleSizeMultiplier"), EffectiveHoleSizeMultiplier);
                MPCInstance->SetScalarParameterValue(TEXT("BlackholeScreenRadius"), ScreenRadiusUV);
                MPCInstance->SetScalarParameterValue(TEXT("HoleRadius"), HoleRadiusUV);
                MPCInstance->SetScalarParameterValue(TEXT("LensRadius"), LensRadiusUV);
                MPCInstance->SetScalarParameterValue(TEXT("BlackholeDepth"), DirToTarget.Size());
            }
        }
    }
}