#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IHttpRequest.h" // Necesario para peticiones web
#include "Asistente.generated.h"
// --- NUEVO: Menú desplegable para elegir el tipo de zona ---
UENUM(BlueprintType)
enum class EClaseDeZona : uint8
{
	Forbidden UMETA(DisplayName = "Zona Prohibida (Regaño)"),
	Edge UMETA(DisplayName = "Orilla/Peligro (Aviso)"),
	Escaleras UMETA(DisplayName = "Maquinaria (Aviso)"),
	Caminandoescalera UMETA(DisplayName = "Caminando en Escalera (Aviso)")
};

// --------------------------------------------------------




UCLASS()
class CHATBOT_VIRTEC_API AAsistente : public AActor
{
	GENERATED_BODY()

public:
	// Constructor por defecto
	AAsistente();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuracion Asistente")
	EClaseDeZona TipoDeZona;

	

protected:
	// Cuando el juego arranca, esto se ejecuta
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere,  Category = "Configuracion de ia")
	FString NombreDeLaZona = TEXT("Zona Desconocida");

public:
	// Tu función principal para mandar el texto a OpenAI
	void Preguntar_IA(const FString& Prompt);

	// La función que "atrapa" la respuesta cuando OpenAI contesta
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);


	// NUEVAS FUNCIONES PARA ELEVENLABS:
	void GenerarAudio_ElevenLabs(const FString& Texto);
	void OnAudioReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	

	// NUEVO: La función que se dispara al chocar (Debe llevar UFUNCTION arriba)
	UFUNCTION()
	void AlEntrarEnLaZona(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// ... tu caja de colisión ...
	UPROPERTY(VisibleAnywhere, Category = "Colision")
	class UBoxComponent* ZonaEquivocada;

	// NUEVO: El altavoz 3D de la IA
	UPROPERTY(VisibleAnywhere, Category = "Audio")
	class UAudioComponent* BocinaIA;
};