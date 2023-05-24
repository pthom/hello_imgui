// An example that shows how to use IndexedDB as a persistent storage under emscripten

#include "hello_imgui/hello_imgui.h"
#include <fstream>

#ifdef EMSCRIPTEN
#include "emscripten.h"
#else
void emscripten_run_script(const char* script) { printf("emscripten_run_script only exists for emscripten!"); }
#endif


// Dummy value that is stored inside /data/DUMMY_VALUE.txt
int DUMMY_VALUE = 0;


// This mounts the IndexedDB at /data
void MountIdbfsData()
{
    std::string script = R"(
        FS.mkdir('/data');
        FS.mount(IDBFS, {}, '/data');

        // Sync IndexedDB => Emscripten
        FS.syncfs(true, function (err) {
            console.log("Sync IndexedDB => Emscripten ended: err= " + err);
        });
    )";
    emscripten_run_script(script.c_str());
}

// Sync Emscripten => IndexedDB
void SyncEmscriptenToIndexDB()
{
    std::string script = R"(
        FS.syncfs(false, function (err) {
            console.log("SyncEmscriptenToIndexDB ended: err= " + err);
        });
    )";
    emscripten_run_script(script.c_str());
}

void WriteDummyValue()
{
    std::ofstream ofs("/data/DUMMY_VALUE.txt");
    if (ofs.good())
        ofs << DUMMY_VALUE;
    else
        fprintf(stderr, "WriteDummyValue: Could not open /data/DUMMY_VALUE.txt");
}

void ReadDummyValue()
{
    std::ifstream ifs("/data/DUMMY_VALUE.txt");
    if (ifs.good())
        ifs >> DUMMY_VALUE;
    else
        fprintf(stderr, "ReadDummyValue: Could not open /data/DUMMY_VALUE.txt");
}


void ShowGui()
{
    ImGui::Text("Hello, IDBFS!");

    ImGui::SliderInt("DUMMY_VALUE", &DUMMY_VALUE, 0, 100);

    ImGui::Text("Mount /data via idbfs (do this first!)");
    if (ImGui::Button("Mount"))
        MountIdbfsData();

    ImGui::Text("ReadDummyValue (read dummy value from IndexedDB)");
    if (ImGui::Button("Read"))
        ReadDummyValue();

    ImGui::Text("WriteDummyValue (write dummy value to IndexedDB)");
    if (ImGui::Button("Write"))
        WriteDummyValue();

    ImGui::Text("SyncEmscriptenToIndexDB (do this before exiting)");
    if (ImGui::Button("Sync"))
        SyncEmscriptenToIndexDB();
}


int main(int , char *[])
{
    HelloImGui::Run(
        ShowGui,
        "IDBFS test", // Window title
        false, false, // Window size auto, windowRestorePreviousGeometry
        {600, 300}    // Window size
    );
    return 0;
}
