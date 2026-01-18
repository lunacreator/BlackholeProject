#include "BlackholeActor.h"

// [필수 헤더]
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Math/RotationMatrix.h"

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

    if (BlackholeMPC)
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return;
        }

        UMaterialParameterCollectionInstance* MPCInstance = World->GetParameterCollectionInstance(BlackholeMPC);
        APlayerController* PC = World->GetFirstPlayerController();

        if (MPCInstance && PC)
        {
            int32 SizeX = 0;
            int32 SizeY = 0;
            PC->GetViewportSize(SizeX, SizeY);

            if (SizeX <= 0 || SizeY <= 0)
            {
                return;
            }

            const FVector Center = MeshComp ? MeshComp->Bounds.Origin : GetActorLocation();
            FVector CamLoc;
            FRotator CamRot;
            PC->GetPlayerViewPoint(CamLoc, CamRot);

            const FVector DirToTarget = Center - CamLoc;
            const float DotProd = FVector::DotProduct(CamRot.Vector(), DirToTarget);
            const bool bIsInFront = DotProd >= 0.0f;

            FVector2D ScreenPos(0.0f, 0.0f);
            const bool bIsOnScreen = PC->ProjectWorldLocationToScreen(Center, ScreenPos, true);

            float UV_X = -10.0f;
            float UV_Y = -10.0f;
            if (bIsInFront && bIsOnScreen)
            {
                UV_X = ScreenPos.X / static_cast<float>(SizeX);
                UV_Y = ScreenPos.Y / static_cast<float>(SizeY);
            }

            float RadiusUV = 0.0f;
            if (bIsInFront && bIsOnScreen && MeshComp)
            {
                const float WorldRadius = MeshComp->Bounds.SphereRadius * FMath::Max(HoleSizeMultiplier, 0.0f);
                if (WorldRadius > 0.0f)
                {
                    const FVector Right = FRotationMatrix(CamRot).GetScaledAxis(EAxis::Y);
                    const FVector EdgeWorld = Center + Right * WorldRadius;

                    FVector2D EdgeScreen(0.0f, 0.0f);
                    if (PC->ProjectWorldLocationToScreen(EdgeWorld, EdgeScreen, true))
                    {
                        const float AspectRatio = static_cast<float>(SizeX) / static_cast<float>(SizeY);
                        FVector2D CenterUV(UV_X, UV_Y);
                        FVector2D EdgeUV(EdgeScreen.X / static_cast<float>(SizeX), EdgeScreen.Y / static_cast<float>(SizeY));
                        FVector2D OffsetUV = EdgeUV - CenterUV;
                        OffsetUV.X *= AspectRatio;
                        RadiusUV = OffsetUV.Size();
                    }
                }
            }

            MPCInstance->SetVectorParameterValue(TEXT("BlackholePos"), FLinearColor(UV_X, UV_Y, 0.0f, RadiusUV));
            MPCInstance->SetScalarParameterValue(TEXT("GravityStrength"), LensingStrength);
        }
    }
}