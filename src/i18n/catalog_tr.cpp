#include "i18n/catalogs.h"

namespace aicpp::i18n {

const std::unordered_map<std::string, std::string>& catalog_tr() {
    static const std::unordered_map<std::string, std::string> table = {
        // common / shared role labels
        {"role.user", "Sen"},
        {"role.assistant", "Asistan"},
        {"role.tool", "Araç"},
        {"role.system", "Sistem"},
        {"common.untitled", "(başlıksız)"},

        // main.cpp
        {"app.banner", "aicpp v0.7 - komutları görmek için /help yaz. Aktif model: {}"},
        {"app.startup_provider_failed", "Başlangıç sağlayıcısı kurulamadı: {}"},
        {"app.bye", "Hoşça kal."},
        {"app.cancelled", "İptal edildi."},
        {"app.turn_failed", "Bu tur beklenmeyen bir hatayla sonlandı, oturum kapanmadı: {}"},
        {"app.cwd_line", "Çalışma dizini: {}"},
        {"app.invalid_cwd", "Geçersiz dizin: {}"},

        // cli/dispatcher.cpp
        {"dispatcher.unknown_command", "Bilinmeyen komut: /{}"},
        {"dispatcher.suggestion", "Bunu mu demek istediniz: /{}?"},
        {"dispatcher.help_hint", "Komut listesi için /help yazabilirsin."},

        // core/permissions/DefaultPermissionManager.cpp
        {"permission.wants_to", "'{}' şu işlemi yapmak istiyor:"},
        {"permission.ask_yna", "İzin ver? (y)es / (n)o / (a)lways bu oturumda: "},

        // tool-call banners (cli/stream_printer.h via cli/theme.cpp)
        {"tool.start", "araç çalışıyor: {}"},
        {"tool.end", "araç tamamlandı: {}"},
        {"tool.end_error", "araç başarısız: {}"},

        // command descriptions/usages
        {"cmd.help.desc", "Kullanılabilir komutları listeler"},
        {"cmd.exit.desc", "Uygulamadan çıkar"},
        {"cmd.clear.desc", "Ekranı ve sohbet geçmişini temizler"},
        {"cmd.model.desc", "Aktif LLM sağlayıcısını/modelini gösterir veya değiştirir"},
        {"cmd.model.usage", "/model [sağlayıcı:model]"},
        {"cmd.tools.desc", "Kullanılabilir araçları ve risk seviyelerini listeler"},
        {"cmd.config.desc", "Config dosyasının yolunu ve içeriğini gösterir"},
        {"cmd.history.desc", "Mevcut oturumu ve geçmiş oturumları listeler"},
        {"cmd.resume.desc", "Geçmiş bir oturuma kaldığı yerden devam eder"},
        {"cmd.resume.usage", "/resume [numara-veya-id]"},
        {"cmd.plan.desc", "Plan modunu aç/kapat (sadece okuma araçları), /plan approve ile uygula"},
        {"cmd.plan.usage", "/plan [metin|approve|cancel]"},
        {"cmd.workflow.desc", "Kayıtlı veya serbest-metin bir workflow'u paralel sub-agent'larla çalıştırır"},
        {"cmd.workflow.usage", "/workflow [list | <isim> <girdi> | <serbest açıklama>]"},
        {"cmd.language.desc", "Arayüz dilini gösterir veya değiştirir"},
        {"cmd.language.usage", "/language [tr|en]"},

        // commands/cmd_help.cpp
        {"help.banner", "aicpp - terminal AI kodlama asistanı"},
        {"help.commands_header", "Komutlar:"},
        {"help.cloud_intro", "Bulut sağlayıcı kullanmak istersen:"},
        {"help.cloud_usage",
         "/model <sağlayıcı>:<model> yaz (örn. openai:gpt-4o-mini, groq:llama-3.3-70b-versatile)."},
        {"help.cloud_autoconfig",
         "Sağlayıcı tanımlı değilse (base URL / API anahtarı eksikse) uygulama senden ister ve\n"
         "config.json'a kaydeder - yeniden başlatmana gerek kalmaz. OpenAI'in chat-completions\n"
         "API'siyle uyumlu her sağlayıcı için geçerli (OpenAI, Groq, Mistral, DeepSeek, Together,\n"
         "Fireworks, xAI/Grok, Azure OpenAI, Gemini'nin openai-uyumlu ucu, vb.)."},
        {"help.language_hint", "Arayüz dilini değiştirmek için /language <tr|en> kullanabilirsin."},
        {"help.footer",
         "Detaylar için /config, geçmiş oturumlar için /history, salt-okunur inceleme için /plan kullanabilirsin."},

        // commands/cmd_clear.cpp
        {"clear.done", "Sohbet geçmişi temizlendi."},

        // commands/cmd_model.cpp
        {"model.setup.explain",
         "'{}' sağlayıcısını şimdi ayarlamak ister misin? Girdiklerin ~/.aicpp/config.json'a\n"
         "kaydedilir, tekrar başlatmana gerek kalmaz. Bu, OpenAI'in chat-completions API'siyle\n"
         "uyumlu HERHANGİ bir sağlayıcı için çalışır (OpenAI, Groq, Mistral, DeepSeek, Together,\n"
         "Fireworks, xAI/Grok, Azure OpenAI, Gemini'nin openai-uyumlu ucu, yerel llama.cpp/LM\n"
         "Studio sunucusu, vb.)."},
        {"model.setup.ask_base_url", "Base URL (örn. https://api.groq.com/openai/v1): "},
        {"model.setup.url_empty", "URL boş, vazgeçildi."},
        {"model.setup.ask_api_key", "API anahtarını yapıştır: "},
        {"model.setup.key_empty", "Anahtar boş, vazgeçildi."},
        {"model.setup.saved_plaintext_warning",
         "Anahtar config.json'a düz metin olarak kaydedildi - bu dosyayı paylaşma."},
        {"model.current", "Mevcut model: {}"},
        {"model.configured_providers", "Yapılandırılmış sağlayıcılar: {}"},
        {"model.switch_hint", "Değiştirmek için: /model <sağlayıcı>:<model>  (örn. /model openai:gpt-4o-mini)"},
        {"model.other_provider_hint",
         "Başka bir sağlayıcı da yazabilirsin (örn. /model groq:llama-3.3-70b-versatile) - "
         "tanımlı değilse sana sorup kaydeder."},
        {"model.change_failed", "Model değiştirilemedi: {}"},
        {"model.changed", "Model değiştirildi: {}"},

        // commands/cmd_tools.cpp
        {"tools.header", "Kullanılabilir araçlar:"},
        {"tools.risk.read", "Read (otomatik izinli)"},
        {"tools.risk.write", "Write (izin gerekir)"},
        {"tools.risk.execute", "Execute (izin gerekir)"},
        {"tools.risk.network", "Network (izin gerekir)"},

        // commands/cmd_config.cpp
        {"config.path", "Config dosyası: {}"},
        {"config.read_failed", "(dosya okunamadı)"},
        {"config.invalid_json", "(dosya geçerli JSON değil)"},
        {"config.redacted_marker", "***gizli*** (görüntüleme için redakte edildi, dosyada düz metin duruyor)"},
        {"config.footer", "Değiştirmek için dosyayı bir editörde düzenleyip uygulamayı yeniden başlat."},

        // commands/cmd_history.cpp
        {"history.current", "Mevcut oturum: {} ({} mesaj, model: {})"},
        {"history.no_other", "Kayıtlı başka oturum yok."},
        {"history.recent_header", "Son oturumlar:"},
        {"history.current_marker", "  <- bu oturum"},
        {"history.resume_hint", "/resume <id> ile bir oturuma devam edebilirsin."},

        // commands/cmd_resume.cpp
        {"resume.none_saved", "Kayıtlı oturum yok."},
        {"resume.list_header", "Geçmiş oturumlar:"},
        {"resume.hint", "Devam etmek için: /resume <numara-veya-id>"},
        {"resume.not_found", "Oturum bulunamadı: {}"},
        {"resume.provider_warning", "Uyarı: kayıtlı model '{}' yüklenemedi ({}), mevcut model korunuyor."},
        {"resume.loaded", "Oturum yüklendi: {} - {} ({} mesaj, model: {})"},
        {"resume.recent_messages_header", "Son mesajlar:"},
        {"resume.grants_restored", "Önceden onaylanmış araçlar bu oturum için geri yüklendi: {}"},

        // commands/cmd_plan.cpp
        {"plan.not_active", "Şu an plan modunda değilsin."},
        {"plan.approved", "Plan onaylandı, uygulamaya geçiliyor..."},
        {"plan.cancelled", "Plan iptal edildi (plan geçmişte kalıyor ama uygulanmayacak)."},
        {"plan.closed", "Plan modu kapatıldı."},
        {"plan.enter_banner",
         "PLAN MODE - sadece okuma araçları çalışır.\n  Onaylamak için: /plan approve   İptal için: /plan cancel"},

        // commands/cmd_workflow.cpp
        {"workflow.none_saved", "Kayıtlı workflow yok."},
        {"workflow.saved_header", "Kayıtlı workflow'lar:"},
        {"workflow.usage_hint", "Kullanım: /workflow <isim> <girdi>   veya   /workflow <serbest açıklama>"},
        {"workflow.meta.no_json",
         "Model geçerli bir JSON spec üretemedi. Küçük/yerel modeller bu adımda güvenilir olmayabilir - "
         "daha güçlü bir /model (örn. openai:...) ile tekrar deneyebilirsin."},
        {"workflow.meta.parse_failed", "Model çıktısı JSON olarak parse edilemedi: {}"},
        {"workflow.meta.too_many_agents", "Üretilen spec 5'ten fazla sub-agent içeriyor, güvenlik için reddedildi."},
        {"workflow.decomposing",
         "serbest açıklama JSON spec'ine çevriliyor (bu bir meta-agent çağrısı)..."},
        {"workflow.run_failed", "Workflow çalıştırılamadı: {}"},
        {"workflow.synthesizing", "tüm aşamalar tamamlandı, sonuçlar birleştiriliyor..."},

        // commands/cmd_language.cpp
        {"language.current", "Mevcut dil: {} ({})"},
        {"language.available", "Kullanılabilir diller: tr (Türkçe), en (English)"},
        {"language.usage_hint", "Değiştirmek için: /language <tr|en>"},
        {"language.invalid", "Geçersiz dil kodu: '{}'. Kullanılabilir: tr, en"},
        {"language.switched", "Dil değiştirildi: {} ({})"},

        // core/tools/LineDiff.cpp
        {"diff.unchanged_lines", "... {} satır değişmedi ..."},
        {"diff.too_large", "Diff gösterilemiyor (dosya çok büyük veya çok farklı)."},
    };
    return table;
}

}  // namespace aicpp::i18n
