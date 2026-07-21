# aicpp

Terminalde çalışan, C++ ile yazılmış bir AI kodlama asistanı. [Claude Code](https://claude.com/claude-code)'un akışından ilham alınarak sıfırdan tasarlandı: slash komutları, tool-use (dosya okuma/yazma/çalıştırma), izin sistemi, plan modu ve paralel sub-agent workflow motoru içerir.

## Özellikler

- **Çoklu LLM sağlayıcı**: Yerelde [Ollama](https://ollama.com) (varsayılan: `qwen2.5-coder:7b`) ve OpenAI'in chat-completions API'siyle uyumlu **herhangi bir bulut sağlayıcı** (OpenAI, Groq, Mistral, DeepSeek, Together, Fireworks, xAI/Grok, Azure OpenAI, Gemini'nin openai-uyumlu ucu, yerel llama.cpp/LM Studio sunucusu, vb.). `/model` ile çalışma anında değiştirilir; tanımsız bir sağlayıcı yazılırsa uygulama gerekli bilgileri (base URL, API anahtarı) sorup kaydeder.
- **Tool-use döngüsü**: `read_file`, `write_file`, `edit_file` (diff-tabanlı), `glob`, `grep` (RE2), `bash` (reproc ile çapraz platform) — okuma otomatik izinli, yazma/çalıştırma kullanıcı onayı gerektirir.
- **Renkli diff önizlemesi**: `write_file`/`edit_file` bir dosyayı değiştirdiğinde, normal sohbet akışında hemen ardından gerçek bir satır-satır diff gösterilir (yeşil eklenen, kırmızı silinen, sadece değişen kısım — tüm dosya değil). Myers algoritmasıyla hesaplanır; büyük dosyalarda/çok farklı içerikte performans için otomatik olarak "diff gösterilemiyor" notuna düşer, uygulamayı yavaşlatmaz.
- **Slash komutları**: `/help /exit /clear /model /tools /config /history /resume /plan /workflow /language`
- **Session kalıcılığı**: `~/.aicpp/` altında config + her turdan sonra otomatik kaydedilen oturumlar, `/resume` ile kaldığı yerden devam. Bir araç için verdiğin "always allow" izinleri de oturumun içinde kalıcı olarak saklanır — `/resume` ile aynı oturuma dönünce daha önce onayladığın araçlar tekrar sormaz.
- **Ctrl-C ile iptal**: AI'dan akan bir cevabı Ctrl-C ile anında durdurabilirsin; uygulama kapanmaz, o ana kadarki kısmi cevap korunur ve "İptal edildi." gösterilir. (Bir araç, örn. `bash`, çalışırken Ctrl-C hâlâ tüm uygulamayı sonlandırır — bu kapsamda değil.)
- **Plan modu**: `/plan` ile salt-okunur inceleme ve plan üretimi, `/plan approve` ile gerçek uygulamaya geçiş.
- **Workflow motoru**: `/workflow` ile kayıtlı ya da serbest-metin görevleri `std::async` tabanlı paralel sub-agent'lara dağıtıp tek bir sentezleme adımında birleştirir.
- **Görsel arayüz**: Kullanıcı girdisi, sistem mesajları ve AI cevapları ANSI renkleriyle görsel olarak ayrışır (asistan cevabı için renkli rol başlığı, araç çağrıları için soluk banner, izin promptu için sarı çerçeve, hata/başarı/uyarı mesajları için kırmızı/yeşil/sarı). `NO_COLOR` ortam değişkenine ve çıktının bir terminale bağlı olup olmadığına (pipe/redirect durumunda otomatik düz metin) saygı gösterir.
- **Çok dilli arayüz (i18n)**: `/language tr` veya `/language en` ile arayüz dili anında değiştirilir (yeniden başlatma gerekmez), `~/.aicpp/config.json`'a kaydedilir. Yeni bir dil eklemek, tek bir katalog dosyası eklemekten ibarettir (bkz. `src/i18n/`).

## Gereksinimler

- Windows 10/11 (birincil hedef; CMake+vcpkg sayesinde Linux/macOS'a taşınabilir)
- [CMake](https://cmake.org/) ≥ 3.25, [Ninja](https://ninja-build.org/)
- MSVC (Visual Studio 2022 Build Tools, "Desktop development with C++" iş yükü) veya uyumlu bir derleyici
- [vcpkg](https://github.com/microsoft/vcpkg) (manifest modu, proje kökündeki `vcpkg.json` bağımlılıkları otomatik kurar: `curl`, `nlohmann-json`, `replxx`, `re2`, `reproc`, `fmt`)
- (Opsiyonel, yerel model için) [Ollama](https://ollama.com)

## Kurulum ve Derleme

```powershell
# vcpkg'ı bir yere klonlayıp bootstrap et (bir kere)
git clone https://github.com/microsoft/vcpkg.git D:\vcpkg
D:\vcpkg\bootstrap-vcpkg.bat

# CMakePresets.json'daki CMAKE_TOOLCHAIN_FILE yolunu kendi vcpkg konumuna gore duzenle,
# sonra:
cmake --preset default
cmake --build build --preset default

# Calistir
.\build\aicpp.exe
```

Tek parça, DLL gerektirmeyen bir **Release** derlemesi için:

```powershell
cmake --preset release-static
cmake --build build-release --preset release-static
# Sonuc: build-release\aicpp.exe
```

VS Code'da çalışmak istersen: **C/C++** ve **CMake Tools** (ikisi de Microsoft) eklentilerini kur, klasörü aç, CMake Tools otomatik `CMakeLists.txt`'i algılar ve build/run/debug arayüzü sağlar.

## Kullanım

```
.\build\aicpp.exe
> /help
```

`/help` mevcut komutları ve bulut sağlayıcı kurulumunu (API anahtarı vb.) nasıl yapacağını gösterir. Arayüz varsayılan olarak Türkçe'dir; İngilizce'ye geçmek için `/language en` yaz.

## Bilinen Sınırlamalar

- GPU hızlandırması olmayan makinelerde Ollama tamamen CPU'da çalışır ve gözle görülür şekilde yavaşlar (Ctrl-C ile en azından akan cevabı hemen durdurabilirsin).
- Küçük/quantize yerel modeller (özellikle 7B ve altı) yapılandırılmış tool-call formatını her zaman güvenilir üretmeyebilir; bu durumlarda daha güçlü bir bulut modeli (`/model openai:...` vb.) daha tutarlı sonuç verir.
- Ctrl-C iptali sadece AI'dan metin akarken çalışır; bir araç (`bash` vb.) çalışırken Ctrl-C basılırsa uygulama yine de tamamen kapanır.

## Proje Durumu

Temel mimari (sağlayıcı soyutlaması, tool sistemi, izinler + kalıcı grantlar, plan modu, workflow motoru, session kalıcılığı, i18n + temalı terminal çıktısı, Ctrl-C ile iptal, renkli diff önizlemesi) tamamlandı ve uçtan uca test edildi. Devam eden/planlanan yönler: ek sağlayıcılar (Anthropic native API), zengin TUI (FTXUI), araç çalışırken (`bash` vb.) de Ctrl-C ile iptal edebilme.
