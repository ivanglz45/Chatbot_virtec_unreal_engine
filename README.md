 Chatbot_virtec_unreal_engine
 Asistente Virtual para Proyecto Virtec

 Descripción del Proyecto
 Este repositorio contiene el código fuente y los *assets* para un asistente virtual interactivo en 3D desarrollado en **Unreal Engine 5**. 

 Al reestructurar la lógica de comunicación a C++ nativo e implementar peticiones HTTP, el sistema logra interacciones fluidas, interactividad espacial 3D y está optimizado para su empaquetado como aplicación móvil y windows.

 Características Principales
 Arquitectura Asíncrona en C++: Peticiones de red sin bloqueos del hilo principal, garantizando una UI/UX completamente fluida.
 Voces de Alta Fidelidad: Integración directa con la API de ElevenLabs para síntesis de voz natural y realista.
 Avatar 3D Realista: Implementación del personaje (tecnología MetaHuman) para la representación espacial del asistente.
 Optimización Móvil: Materiales, iluminación estática y colisiones optimizadas para compilación fluida en formato APK para Android.

 Herramientas y Tecnologías
 Motor Gráfico: Unreal Engine 5.
 Desarrollo:C++ nativo y Blueprints
 Control de Versiones: Git LFS (Large File Storage)
 APIs de IA: ElevenLabs (Text-to-Speech) / OpenAI


 Requisitos Previos y Obtención de APIs
 Para que otros desarrolladores puedan clonar y probar este proyecto localmente, es necesario configurar credenciales propias de las APIs utilizadas.

 1. ElevenLabs API Key: Esencial para generar el audio dinámico. Puedes obtener una clave de desarrollo gratuita registrándote en [ElevenLabs](https://elevenlabs.io/).
 2. OpenAI api key: Necesaria para el procesamiento de lenguaje natural y la lógica del chatbot. Puedes generar tu clave desde la [Plataforma de Desarrolladores de OpenAI](https://platform.openai.com/api-keys).
 3. Git LFS: Es estrictamente necesario tener instalado [Git LFS](https://git-lfs.com/) antes de clonar el repositorio para descargar correctamente el modelo del MetaHuman (`.dna`), las texturas (`.uasset`) y los mapas (`.umap`).



 Instalación y Uso Rápido

 # 1. Clonar el repositorio (Asegúrate de tener Git LFS instalado y activo)
 git clone (https://github.com/ivanglz45/Chatbot_virtec_unreal_engine.git)

# 2. Generar archivos de proyecto
 Ve a la carpeta raíz, haz clic derecho sobre "Chatbot_Virtec.uproject" y selecciona "Generate Visual Studio / CMake project files".

# 3. Abrir y compilar
 Abre el proyecto en Unreal Engine. Compila el código fuente C++ desde tu IDE preferido (o directamente en el Live Coding del motor).

# 4. Configurar las APIs
 Localiza la clase principal del chatbot e introduce tus propias API Keys en los campos correspondientes antes de darle a Play.
