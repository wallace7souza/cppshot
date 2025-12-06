#ifndef CPPSHOT_SCREENSHOTEDITORFRAME_H
#define CPPSHOT_SCREENSHOTEDITORFRAME_H

// --- Inclusões de Headers (apenas o necessário para as declarações) ---
#include <wx/image.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/choice.h> // Se necessário para membros, mas não parece ser aqui
#include <wx/filedlg.h> // Se necessário para membros, mas não parece ser aqui
#include <wx/sizer.h> // Se necessário para membros, mas não parece ser aqui
#include <wx/menu.h>
#include <wx/string.h> // Para wxString
#include <wx/gdicmn.h> // Para wxPoint, wxRect, wxSize

// --- IDs de Comandos do Menu ---
enum {
    ID_CROP = 100,
    ID_BLUR = 101,
    ID_PIXELATE = 102,
    ID_COPY = 103,

    ID_DRAW_ARROW=201,
    ID_DRAW_RECTANGLE=201,

};

// =========================================================
// Classe do Frame (Janela) do Editor de Screenshot
// =========================================================
class ScreenshotEditorFrame : public wxFrame {
public:
    // Construtor (apenas a assinatura)
    ScreenshotEditorFrame(const wxString& title, const wxImage& initialImage);

private:
    // --- Membros de Dados ---
    wxImage m_currentImage;
    wxBitmap m_bitmap;
    wxPanel* m_panel = nullptr;
    wxPoint m_startPoint;
    wxRect m_selectionRect;
    bool m_isSelecting = false;

    // --- Protótipos dos Handlers de Eventos (Assinaturas) ---
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnRightClick(wxMouseEvent& event);
    void OnMenuSelection(wxCommandEvent& event);

    // --- Protótipos dos Métodos de Lógica de Edição (Assinaturas) ---
    void ApplyCrop();
    void ApplyBlur(int radius);
    void ApplyPixelate(int blockSize);
    void CopyToClipboard() const;
    void SaveImage();
};

#endif // CPPSHOT_SCREENSHOTEDITORFRAME_H