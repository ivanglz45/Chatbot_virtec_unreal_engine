#include "Asistente.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "Engine/Engine.h"

AAsistente::AAsistente()
{
	PrimaryActorTick.bCanEverTick = false; // No necesitamos actualizarlo cada frame


	// 1. Creamos la caja invisible
	ZonaEquivocada = CreateDefaultSubobject<UBoxComponent>(TEXT("ZonaEquivocada"));
	RootComponent = ZonaEquivocada; // La hacemos la base del objeto

	// 2. Le damos un tamaño inicial (100x100x100 unidades)
	ZonaEquivocada->InitBoxExtent(FVector(100.0f, 100.0f, 100.0f));

	// 3. Le decimos que actúe como un "Trigger" (detector fantasma)
	ZonaEquivocada->SetCollisionProfileName(TEXT("Trigger"));

	// 4. Conectamos la caja con nuestra función AlEntrarEnLaZona
	ZonaEquivocada->OnComponentBeginOverlap.AddDynamic(this, &AAsistente::AlEntrarEnLaZona);
	// ... código de la caja ...

	// 5. Creamos la bocina 3D
	BocinaIA = CreateDefaultSubobject<UAudioComponent>(TEXT("BocinaIA"));
	BocinaIA->SetupAttachment(RootComponent); // La pegamos a la caja
	BocinaIA->bAutoActivate = false; // Que no suene hasta que se lo ordenemos
}

void AAsistente::BeginPlay()
{
	Super::BeginPlay();

	// Vamos a hacer que Paco o el Asistente se presenten nada más darle a Play
	//Preguntar_IA(TEXT("Hola, eres el nuevo asistente virtual. Escribe una frase corta y sarcástica presentándote."));
}

void AAsistente::Preguntar_IA(const FString& Prompt)
{
	// 1. Tu clave de API directa
	FString ApiKey = TEXT("");

	// 2. Preparamos la conexión nativa de Unreal
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AAsistente::OnResponseReceived);
	Request->SetURL("https://api.openai.com/v1/chat/completions");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", "Bearer " + ApiKey);

	// 3. Armamos el JSON para gpt-4o-mini
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("model", "gpt-4o-mini");

	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	TSharedPtr<FJsonObject> MessageObject = MakeShareable(new FJsonObject());
	MessageObject->SetStringField("role", "user");
	MessageObject->SetStringField("content", Prompt);
	MessagesArray.Add(MakeShareable(new FJsonValueObject(MessageObject)));

	JsonObject->SetArrayField("messages", MessagesArray);

	// Convertimos el objeto JSON a texto
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// 4. Enviamos la petición sin congelar el juego
	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();
}

void AAsistente::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (bConnectedSuccessfully && Response.IsValid())
	{
		// 5. Desarmamos el JSON que nos regresó OpenAI
		TSharedPtr<FJsonObject> ResponseObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, ResponseObj))
		{
			const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
			// Buscamos "choices[0].message.content"
			if (ResponseObj->TryGetArrayField("choices", ChoicesArray) && ChoicesArray->Num() > 0)
			{
				TSharedPtr<FJsonObject> MessageObj = (*ChoicesArray)[0]->AsObject()->GetObjectField("message");
				FString TextoRespuesta = MessageObj->GetStringField("content");

				// 6. ¡Lo imprimimos en la consola de Unreal!
				UE_LOG(LogTemp, Warning, TEXT("🤖 Asistente IA dice: %s"), *TextoRespuesta);

				GenerarAudio_ElevenLabs(TextoRespuesta);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Error al conectar con OpenAI"));
	}
}

void AAsistente::GenerarAudio_ElevenLabs(const FString& Texto)
{
	FString ElevenApiKey = TEXT("");

	// Usaremos el ID de la voz de Antoni
	FString VoiceID = TEXT("ErXwobaYiN019PkySvjV");

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AAsistente::OnAudioReceived);
	Request->SetURL("https://api.elevenlabs.io/v1/text-to-speech/" + VoiceID + "?output_format=pcm_24000");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("xi-api-key", ElevenApiKey);

	// Armamos el JSON para ElevenLabs
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("text", Texto);
	JsonObject->SetStringField("model_id", "eleven_multilingual_v2");

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();

	UE_LOG(LogTemp, Warning, TEXT("⏳ Solicitando audio a ElevenLabs..."));
}

void AAsistente::OnAudioReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (bConnectedSuccessfully && Response.IsValid())
	{
		// 1. Obtenemos los datos crudos (ya no es un MP3)
		TArray<uint8> AudioBytes = Response->GetContent();

		if (AudioBytes.Num() > 0)
		{
			// 2. Creamos una "onda de sonido virtual" en la memoria RAM
			USoundWaveProcedural* AudioProcedural = NewObject<USoundWaveProcedural>();
			AudioProcedural->SetSampleRate(24000); // Debe coincidir con lo que le pedimos a ElevenLabs
			AudioProcedural->NumChannels = 1;      // Audio Mono (para que funcione el 3D)
			AudioProcedural->Duration = INDEFINITELY_LOOPING_DURATION;
			AudioProcedural->SoundGroup = SOUNDGROUP_Voice;
			AudioProcedural->bLooping = false;

			// 3. Le inyectamos los bytes directamente
			AudioProcedural->QueueAudio(AudioBytes.GetData(), AudioBytes.Num());

			// 4. Se lo pasamos a nuestra bocina y... ¡Play!
			BocinaIA->SetSound(AudioProcedural);
			BocinaIA->Play();

			UE_LOG(LogTemp, Warning, TEXT("🔊 ¡El Asistente está hablando en vivo dentro del juego!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Error al descargar el audio de ElevenLabs"));
	}
}

void AAsistente::AlEntrarEnLaZona(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !OtherActor || OtherActor == this)
	{
		return;
	}
	// Verificamos que el objeto exista y que no sea el propio Asistente chocando consigo mismo
	
		FString PromptFinal;
		FString MensajePantalla;

		// Obtenemos el nombre del objeto o jugador que entró
		switch (TipoDeZona)
		{
		case EClaseDeZona::Forbidden:
			// Lógica para Zona Prohibida
			MensajePantalla = TEXT("¡ZONA PROHIBIDA!");
			PromptFinal = TEXT("Imagina que eres un supervisor robótico sarcástico de Virtec. El usuario va a subir la escalera dale la advertencia si sube las escaleras que se cuide de las orillas. Dame UNA frase breve, única y con cuidado.");
			break;

		case EClaseDeZona::Edge:
			// Lógica para Orilla de Peligro
			MensajePantalla = TEXT("¡CUIDADO: ORILLA!");
			PromptFinal = TEXT("Imagina que eres un supervisor robótico de seguridad de Virtec preocupado por la seguridad. El usuario  se acercó peligrosamente a una orilla o precipicio. Dale un aviso urgente y directo como '¡Cuidado!' o '¡Peligro, orilla!' para que retroceda inmediatamente.");
			break;
		case EClaseDeZona::Escaleras:
			MensajePantalla = TEXT("¡CUIDADO: ESCALERAS!");
			PromptFinal = TEXT("Imagina que eres un supervisor robótico. El usuario está en las escaleras, dile que tenga cuidado. por que no hay pasamanos y que sea corto la frase");
			break;
		case EClaseDeZona::Caminandoescalera:
		MensajePantalla = TEXT("¡CUIDADO: CAMINANDO EN ESCALERAS!");
				PromptFinal = TEXT("Imagina que eres un supervisor robótico. El usuario está caminando en las escaleras, dile que tenga cuidado. por que no hay pasamanos y que no corra que sea corto la frase");
				break;
				
		default:
			break;
		}

		// ---------------------------------------------------------

		// 1. Mostramos el texto en la pantalla del juego
		if (GEngine)
		{
			// Si es orilla usamos Naranja, si es prohibida usamos Rojo
			FColor ColorAlerta = (TipoDeZona == EClaseDeZona::Edge) ? FColor::Orange : FColor::Red;
			GEngine->AddOnScreenDebugMessage(-1, 5.f, ColorAlerta, MensajePantalla);
		}

		UE_LOG(LogTemp, Warning, TEXT("🚨 %s: Avisando a la IA..."), *MensajePantalla);

		// 2. Enviamos el prompt específico a OpenAI para que el asistente hable
		Preguntar_IA(PromptFinal);
	}



