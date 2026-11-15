# Kawaii Hardcore — синтезатор для FL Studio (VST3)

Gabber / happy hardcore / old school / trance, звук в духе S3RL.
Исходники на JUCE 7 (C++). Нужно один раз собрать в .vst3.

## Вариант A — без установки компилятора (GitHub Actions)
1. Создай пустой репозиторий на GitHub, залей туда содержимое этой папки (включая .github).
2. Вкладка Actions -> "Build VST3 (Windows)" -> Run workflow.
3. Скачай артефакт KawaiiHardcore-VST3 (внутри папка "Kawaii Hardcore.vst3").

## Вариант B — локально (Windows)
Нужны: Visual Studio 2022 (workload "Desktop development with C++"), CMake, Git.

    cmake -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release --target KawaiiHardcore_VST3

Результат: build\KawaiiHardcore_artefacts\Release\VST3\Kawaii Hardcore.vst3

## Установка в FL Studio
1. Скопируй "Kawaii Hardcore.vst3" в C:\Program Files\Common Files\VST3\
2. FL Studio -> Options -> Manage plugins -> Find installed plugins.
3. Add -> Kawaii Hardcore (в разделе Generators).

## Что внутри
- Supersaw до 7 голосов (PolyBLEP, detune, stereo width), Pulse, Sine
- Resonant lowpass + filter envelope, ADSR
- Pitch Drop (для габбер-кика и hoover-нырка), Drive (tanh), Trance Gate (синхрон с темпом хоста)
- 10 пресетов: S3RL Supersaw Lead, Old School Hoover, Gabber Kick, Happy Core Bass,
  Trance Pluck, Trance Gate Pad, Euro Stab, Rave Screech Lead, Distorted Gabber Lead, Kawaii Soft Pad
