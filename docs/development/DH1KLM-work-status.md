# Lokaler Arbeitsstand

- Projekt: DH1KLM/NereusSDR; Uploadbranch: `DH1KLM-NereusSDR`.
- Thetis ist maßgebliche Referenz, AetherSDR ergänzend.
- Vorhandene Windows-Umgebung weiterverwenden; gezielte Prüfungen, vollständiger Anwendungsbuild vor Upload. Commits signieren. Neue Code-Kommentare mit `//DH1KLM` kennzeichnen.
- Buildskript: `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/windows/build-nereus.ps1`.
- Abgeleitet aus `D:\GitHub\set-Nereusenv.ps1`, mit repositoryrelativem Projektpfad. Standard: MSVC 18 Insiders x64, Qt 6.8.3, Ninja, vier parallele Jobs. Pfade und Jobzahl sind Parameter.
- Der alte Cache in `build/` verweist auf `C:/Users/DH1KLM/Documents/GitHub/NereusSDR` und ist am aktuellen Standort nicht nutzbar. Er bleibt erhalten; Standardziel ist `build-windows/` mit `RelWithDebInfo`. Abhängigkeitsquellen und Opus-Archive aus `build/` werden lokal wiederverwendet; diese Dateien müssen vorhanden bleiben. Gebaut werden GUI, Daemon und Audio-Capture samt Abhängigkeiten; Tests werden nicht pauschal mitgebaut.
- ASIO wird mit dem mitgelieferten SDK gebaut. Die optionale DeepFilterNet3-Bibliothek fehlt und bleibt deaktiviert.
- `/bigobj` behebt das MSVC-Objektformatlimit bei `RadioModel.cpp`. Folgebuilds konfigurieren nicht erneut; bei Bedarf `-Reconfigure` verwenden.
- `PinnedRadioWait` trägt die vorhandene `NEREUS_CORE_EXPORT`-Markierung, damit sein Qt-Metaobjekt beim Windows-Linken der GUI importiert wird.
- Geprüft am 22.10.2026: vollständiger lokaler Anwendungsbuild (NereusSDR, nereusd, nereus-audio-capture), anschließender inkrementeller Skriptaufruf und `git diff --check` erfolgreich. Vorhandene Compiler-/Linkerwarnungen bleiben. Kein GUI-/Hardware-Funktionstest und kein vollständiger Test-Suite-Lauf.
- Keine externen Quellen ohne Auftrag (GitHub ausgenommen); keine AetherSDR-Buildskripte für NereusSDR verwenden.
