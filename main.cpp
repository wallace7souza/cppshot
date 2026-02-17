#include <wx/display.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include "App.h"
#include "ScreenshotEditorFrame.h"

// Inicialização da aplicação
bool MyApp::OnInit() {
    // Inicializa handlers de imagem para JPG/PNG etc.
    wxImage::AddHandler(new wxPNGHandler);
    wxImage::AddHandler(new wxJPEGHandler);

    RestartCapture();

    return true;
}

// 4. Lógica de Captura
void MyApp::RestartCapture() {
    // 1. Obter informações dos monitores
    int numDisplays = wxDisplay::GetCount();

    if (numDisplays == 0) {
        wxMessageBox("Nenhum monitor detectado.", "Erro", wxOK | wxICON_ERROR);
        return;
    }

    int selectedDisplayIndex = 0;

    // 2. Se houver mais de um, pedir a seleção
    if (numDisplays > 1) {
        wxArrayString choices;
        for (int i = 0; i < numDisplays; ++i) {
            wxDisplay display(i);
            wxRect rect = display.GetGeometry();
            choices.Add(wxString::Format("Monitor %d (%dx%d)", i, rect.GetWidth(), rect.GetHeight()));
        }

        wxString selection = wxGetSingleChoice("Escolha qual monitor capturar:",
                                               "Seleção de Monitor",
                                               choices);

        if (selection.IsEmpty()) return; // Usuário cancelou

        // Encontra o índice selecionado
        selectedDisplayIndex = choices.Index(selection);
    }

    // 3. Captura de tela e lançamento do editor
    CaptureAndLaunchEditor(selectedDisplayIndex);
}

void MyApp::CaptureAndLaunchEditor(int displayIndex) {
    wxDisplay display(displayIndex);
    wxRect rect = display.GetGeometry();

    // Criação do Device Context da Tela
    wxScreenDC dcScreen;
    if (!dcScreen.IsOk()) {
        wxMessageBox("Não foi possível acessar a tela (ScreenDC).", "Erro", wxOK | wxICON_ERROR);
        return;
    }

    // Criação do Bitmap de destino
    wxBitmap bmp(rect.width, rect.height);
    wxMemoryDC dcMem;
    dcMem.SelectObject(bmp); // Associa o bitmap ao DC de memória

    // Captura (Blit copia da Tela para o Bitmap)
    dcMem.Blit(0, 0, rect.width, rect.height,
               &dcScreen,
               rect.x, rect.y);

    dcMem.SelectObject(wxNullBitmap); // Desassocia

    // Converte wxBitmap para wxImage (necessário para edição)
    wxImage image = bmp.ConvertToImage();

    if (!image.IsOk()) {
        wxMessageBox("Erro ao converter a captura para imagem editável.", "Erro", wxOK | wxICON_ERROR);
        return;
    }

    // 5. Abrir a Janela de Edição
    ScreenshotEditorFrame* frame = new ScreenshotEditorFrame("Editor de Screenshot (wxWidgets)", image);
    frame->Show(true);
}

// Macro para iniciar a aplicação wxWidgets
wxIMPLEMENT_APP(MyApp);