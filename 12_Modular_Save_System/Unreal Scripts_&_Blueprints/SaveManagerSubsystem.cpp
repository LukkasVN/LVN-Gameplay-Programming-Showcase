#include "SaveManagerSubsystem.h"
#include "SaveGameData.h"
#include "SaveableComponent.h"
#include "GUIDComponent.h"
#include "SaveableTransformComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

void USaveManagerSubsystem::SaveGame()
{
    USaveGameData* SaveData = Cast<USaveGameData>(
        UGameplayStatics::CreateSaveGameObject(USaveGameData::StaticClass())
    );

    UWorld* World = GetWorld();
    if (!World) return;

    for (TObjectIterator<USaveableComponent> It; It; ++It)
    {
        USaveableComponent* Saveable = *It;
        if (!Saveable) continue;

        // Only components in this world
        if (Saveable->GetWorld() != World) continue;

        // Skip class defaults / templates
        if (Saveable->HasAnyFlags(RF_ClassDefaultObject)) continue;

        AActor* Owner = Saveable->GetOwner();
        if (!Owner) continue;

        UGUIDComponent* GUIDComp = Owner->FindComponentByClass<UGUIDComponent>();
        if (!GUIDComp) continue;

        FSaveDataEntry Entry;
        Entry.GUID = GUIDComp->GUID;
        Entry.Type = Saveable->GetSaveDataType();
        Entry.JsonData = Saveable->CaptureState();

        SaveData->Entries.Add(Entry);
    }

    UGameplayStatics::SaveGameToSlot(SaveData, TEXT("MainSave"), 0);
    UE_LOG(LogTemp, Warning, TEXT("Saving %d entries"), SaveData->Entries.Num());
}

void USaveManagerSubsystem::LoadGame()
{
    USaveGameData* SaveData = Cast<USaveGameData>(
        UGameplayStatics::LoadGameFromSlot(TEXT("MainSave"), 0)
    );
    if (!SaveData) return;

    UWorld* World = GetWorld();
    if (!World) return;

    for (TObjectIterator<USaveableComponent> It; It; ++It)
    {
        USaveableComponent* Saveable = *It;
        if (!Saveable) continue;

        // Only components in this world
        if (Saveable->GetWorld() != World) continue;

        // Skip class defaults / templates
        if (Saveable->HasAnyFlags(RF_ClassDefaultObject)) continue;

        AActor* Owner = Saveable->GetOwner();
        if (!Owner) continue;

        UGUIDComponent* GUIDComp = Owner->FindComponentByClass<UGUIDComponent>();
        if (!GUIDComp) continue;

        for (const FSaveDataEntry& Entry : SaveData->Entries)
        {
            if (Entry.GUID == GUIDComp->GUID)
            {
                Saveable->RestoreState(Entry.JsonData);
                break;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Loaded %d entries"), SaveData->Entries.Num());
}

void USaveManagerSubsystem::DeleteSaveGame()
{
    UGameplayStatics::DeleteGameInSlot(TEXT("MainSave"), 0);
}

void USaveManagerSubsystem::ResetAllToDefault()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TObjectIterator<USaveableComponent> It; It; ++It)
    {
        USaveableComponent* Saveable = *It;
        if (!Saveable) continue;

        // Only components in this world
        if (Saveable->GetWorld() != World) continue;

        // Skip class defaults / templates
        if (Saveable->HasAnyFlags(RF_ClassDefaultObject)) continue;

        if (USaveableTransformComponent* TransformComp = Cast<USaveableTransformComponent>(Saveable))
        {
            TransformComp->ResetToDefault();
        }

        /* Example for further implementations
        if (USaveableCustomComponent* CustomComp = Cast<USaveableCustomComponent>(Saveable))
        {
            CustomComp->CustomResetMethod();
        }
        */
        
    }
}
