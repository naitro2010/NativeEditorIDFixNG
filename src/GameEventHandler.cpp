#include "GameEventHandler.h"
#include "Hooks.h"
#pragma warning(disable : 4091)
#pragma warning(disable : 4189)
namespace plugin {
    typedef struct FormEditorIDEntry {
        RE::FormID formId = 0;
        RE::BSFixedString editorId;
        bool operator==(const FormEditorIDEntry& other) const noexcept {
            return other.formId == this->formId;
        };
        bool operator==(RE::FormID otherId) const noexcept {
            return otherId == this->formId;
        }
        std::weak_ordering operator<=>(const FormEditorIDEntry &other) const noexcept {
            return this->formId <=> other.formId;
        }
        std::weak_ordering operator<=>(RE::FormID otherId) const noexcept {
            return this->formId <=> otherId;
        }
    } FormEditorIDEntry;
    std::mutex table_mutex;
    std::vector<std::unordered_map<RE::FormID,FormEditorIDEntry>> EditorIdLookupTables;
    static bool NewSetEditorID(RE::TESForm& form, const char* name) {
        if (!name || name[0] == '\x00') {
            return true;
        }
        if (strnlen(name, 127) != 127) {
            std::lock_guard l(table_mutex);
            auto& table = EditorIdLookupTables[form.formType.underlying()];
            RE::BSFixedString editorId(name);
            FormEditorIDEntry entry{form.formID, editorId};
            table.insert_or_assign(form.formID, entry);

        } else {
            std::lock_guard l(table_mutex);
            auto& table = EditorIdLookupTables[form.formType.underlying()];
            std::string cutName;
            cutName.assign(name, 127);
            RE::BSFixedString editorId(cutName.c_str());
            FormEditorIDEntry entry{form.formID, editorId};
            table.insert_or_assign(form.formID, entry);
        }
        return true;
    }
    static const char* NewGetEditorID(RE::TESForm& form) {
        std::lock_guard l(table_mutex);
        auto& table = EditorIdLookupTables[form.formType.underlying()];
        if (!table.contains(form.formID)) {
            return "";
        }
        return table.find(form.formID)->second.editorId.c_str();
    }
    uintptr_t OriginalGetEditorID = 0x0;
    uintptr_t OriginalSetEditorID = 0x0;
    void GameEventHandler::onLoad() {
        logger::info("onLoad()");
        Hooks::install();
        if (REL::Module::IsVR()) {
            EditorIdLookupTables.reserve(0x100);
            OriginalGetEditorID = REL::Offset(0x107430).address();
            OriginalSetEditorID = REL::Offset(0x107890).address();
            auto OriginalNodeStringAddr = REL::Offset(0x15bcc88).address();
            auto OriginalNullStringAddr = REL::Offset(0x15965f0).address();
            auto StringPatchA0 = REL::Offset(0x1d8f49).address();
            auto StringPatchA1 = REL::Offset(0x1d960e).address();
            uint8_t A0Data[] = {0x44, 0x8b, 0xCB};
            REL::safe_write(StringPatchA0, A0Data, 3);
            auto StringPatchB0 = REL::Offset(0x1d9b48).address();
            uint8_t A1Data[] = {0x44,0x8b,0xCF};
            REL::safe_write(StringPatchA1, A1Data, 3);
            uint8_t B0Data[] = {0x90,0x90,0x90,0x90,0x90,0x89,0x5c,0x24,0x20};
            REL::safe_write(StringPatchB0, B0Data,9);
            REL::safe_write(OriginalNodeStringAddr, "%s  (%08X)\x00", 11);
            auto StringPatchC0 = REL::Offset(0x1d7b6b).address();
            uint8_t C0Data[] = {0x48,0xbf,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x90,0x90};
            memcpy(&C0Data[2], &OriginalNullStringAddr, 8);
            REL::safe_write(StringPatchC0, C0Data, sizeof(C0Data));
            auto StringPatchC1 = REL::Offset(0x1d7b81).address();
            uint8_t C1Data[] = {0x49,0xb9,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x90,0x90,0x90,0x90,0x90};
            memcpy(&C1Data[2], &OriginalNullStringAddr, 8);
            REL::safe_write(StringPatchC1, C1Data, sizeof(C1Data));
            auto StringPatchD0 = REL::Offset(0x1da6b6).address();
            uint8_t D0Data[] = {0x49, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x90, 0x90};
            memcpy(&D0Data[2], &OriginalNullStringAddr, 8);
            REL::safe_write(StringPatchD0, D0Data, sizeof(D0Data));
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&) OriginalGetEditorID, NewGetEditorID);
            DetourTransactionCommit();
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&) OriginalSetEditorID, NewSetEditorID);
            DetourTransactionCommit();
        }
    }

    void GameEventHandler::onPostLoad() {
        logger::info("onPostLoad()");
    }

    void GameEventHandler::onPostPostLoad() {
        logger::info("onPostPostLoad()");
    }

    void GameEventHandler::onInputLoaded() {
        logger::info("onInputLoaded()");
    }

    void GameEventHandler::onDataLoaded() {
        logger::info("onDataLoaded()");
    }

    void GameEventHandler::onNewGame() {
        logger::info("onNewGame()");
    }

    void GameEventHandler::onPreLoadGame() {
        logger::info("onPreLoadGame()");
    }

    void GameEventHandler::onPostLoadGame() {
        logger::info("onPostLoadGame()");
    }

    void GameEventHandler::onSaveGame() {
        logger::info("onSaveGame()");
    }

    void GameEventHandler::onDeleteGame() {
        logger::info("onDeleteGame()");
    }
}  // namespace plugin