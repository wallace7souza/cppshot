#include "ScreenshotEditorFrame.h"
#include <wx/dcclient.h>
#include <wx/msgdlg.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/button.h>
#include <algorithm> // Para std::min e std::abs
#include <iostream> // Para debug, se necessário
#include <wx/wfstream.h> // Pode ser útil para SaveFile, dependendo da plataforma
#include <wx/log.h> // Para WXUNUSED

// =========================================================
// Implementação do Construtor
// =========================================================
ScreenshotEditorFrame::ScreenshotEditorFrame(const wxString& title, const wxImage& initialImage)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, initialImage.GetSize()) {

    // Inicializa a imagem e o bitmap
    m_currentImage = initialImage;
    m_bitmap = wxBitmap(m_currentImage);

    // Cria um painel principal
    m_panel = new wxPanel(this, wxID_ANY);

    // Cria um sizer para o frame para adicionar margens ao painel
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);

    // Barra de ferramentas simples para cores
    wxBoxSizer* colorSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton* btnYellow = new wxButton(this, ID_COLOR_YELLOW, "", wxDefaultPosition, wxSize(30, 30));
    btnYellow->SetBackgroundColour(*wxYELLOW);
    
    wxButton* btnBlack = new wxButton(this, ID_COLOR_BLACK, "", wxDefaultPosition, wxSize(30, 30));
    btnBlack->SetBackgroundColour(*wxBLACK);
    
    wxButton* btnRed = new wxButton(this, ID_COLOR_RED, "", wxDefaultPosition, wxSize(30, 30));
    btnRed->SetBackgroundColour(*wxRED);
    
    wxButton* btnWhite = new wxButton(this, ID_COLOR_WHITE, "", wxDefaultPosition, wxSize(30, 30));
    btnWhite->SetBackgroundColour(*wxWHITE);

    colorSizer->Add(btnYellow, 0, wxALL, 2);
    colorSizer->Add(btnBlack, 0, wxALL, 2);
    colorSizer->Add(btnRed, 0, wxALL, 2);
    colorSizer->Add(btnWhite, 0, wxALL, 2);

    frameSizer->Add(colorSizer, 0, wxALIGN_CENTER | wxTOP, 5);
    frameSizer->Add(m_panel, 1, wxEXPAND | wxALL, 10); // Margem de 10 pixels
    SetSizer(frameSizer);

    // Bindings dos eventos
    m_panel->Bind(wxEVT_PAINT, &ScreenshotEditorFrame::OnPaint, this);
    m_panel->Bind(wxEVT_LEFT_DOWN, &ScreenshotEditorFrame::OnLeftDown, this);
    m_panel->Bind(wxEVT_MOTION, &ScreenshotEditorFrame::OnMouseMove, this);
    m_panel->Bind(wxEVT_LEFT_UP, &ScreenshotEditorFrame::OnLeftUp, this);
    m_panel->Bind(wxEVT_RIGHT_DOWN, &ScreenshotEditorFrame::OnRightClick, this);

    // Bind único para eventos de menu
    Bind(wxEVT_MENU, &ScreenshotEditorFrame::OnMenuSelection, this, wxID_ANY);
    
    // Bind para botões de cores
    Bind(wxEVT_BUTTON, &ScreenshotEditorFrame::OnColorSelect, this, ID_COLOR_YELLOW);
    Bind(wxEVT_BUTTON, &ScreenshotEditorFrame::OnColorSelect, this, ID_COLOR_BLACK);
    Bind(wxEVT_BUTTON, &ScreenshotEditorFrame::OnColorSelect, this, ID_COLOR_RED);
    Bind(wxEVT_BUTTON, &ScreenshotEditorFrame::OnColorSelect, this, ID_COLOR_WHITE);

    // Define o tamanho inicial da janela e centraliza
    // Calcula o tamanho necessário: imagem + margens + barra de cores
    wxSize sizeWithMargins = m_currentImage.GetSize();
    sizeWithMargins.IncBy(20, 60); // Aumentado para acomodar os botões
    SetClientSize(sizeWithMargins);
    Centre();
}

// =========================================================
// Implementação dos Handlers de Eventos
// =========================================================

void ScreenshotEditorFrame::OnPaint(wxPaintEvent& WXUNUSED(event)) {
    wxPaintDC dc(m_panel);

    // Desenha a imagem atual
    dc.DrawBitmap(m_bitmap, 0, 0);

    // Desenha o retângulo de seleção se estiver ativo
    if (!m_selectionRect.IsEmpty()) {
        dc.SetPen(wxPen(m_currentColor, 2));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(m_selectionRect);
    }
}

void ScreenshotEditorFrame::OnLeftDown(wxMouseEvent& event) {
    m_isSelecting = true;
    m_startPoint = event.GetPosition();
    m_selectionRect.SetSize({0, 0});
    m_panel->Refresh(); // Redesenha para limpar a seleção anterior
}

void ScreenshotEditorFrame::OnMouseMove(wxMouseEvent& event) {
    if (!m_isSelecting) return;

    wxPoint endPoint = event.GetPosition();

    // Cria o retângulo normalizado
    int x = std::min(m_startPoint.x, endPoint.x);
    int y = std::min(m_startPoint.y, endPoint.y);
    int w = std::abs(endPoint.x - m_startPoint.x);
    int h = std::abs(endPoint.y - m_startPoint.y);
    m_selectionRect = wxRect(x, y, w, h);

    m_panel->Refresh(false); // Refresca, mas não apaga o fundo
}

void ScreenshotEditorFrame::OnLeftUp(wxMouseEvent& event) {
    m_isSelecting = false;

    // Validação de seleção mínima
    if (m_selectionRect.GetWidth() < 5 || m_selectionRect.GetHeight() < 5) {
        m_selectionRect.SetSize(wxSize(0, 0)); // Seleção inválida, limpa o retângulo
        m_panel->Refresh();
    }
}

void ScreenshotEditorFrame::OnRightClick(wxMouseEvent& event) {
    // Criação do menu de contexto
    wxMenu menu;

    if (m_selectionRect.IsEmpty()) {
        menu.Append(ID_COPY, "Copiar");
        menu.AppendSeparator();
        menu.Append(wxID_SAVE, "Salvar Imagem");
    } else {
        menu.Append(ID_CROP, "Cortar (Crop)");
        menu.Append(ID_BLUR, "Desfoque (Blur)");
        menu.Append(ID_PIXELATE, "Pixelizar");
        menu.Append(ID_COPY, "Copiar");
        menu.AppendSeparator();
        menu.Append(wxID_SAVE, "Salvar Imagem");
    }

    // Exibe o menu na posição do clique
    m_panel->PopupMenu(&menu, event.GetPosition());
}

void ScreenshotEditorFrame::OnMenuSelection(wxCommandEvent& event) {
    switch (event.GetId()) {
        case ID_CROP:
            ApplyCrop();
            break;
        case ID_BLUR:
            ApplyBlur(23);
            break;
        case ID_PIXELATE:
            ApplyPixelate(20);
            break;
        case ID_COPY:
            CopyToClipboard();
            break;
        case wxID_SAVE:
            SaveImage();
            break;
    }

    // Após a edição, limpa a seleção
    m_selectionRect.SetSize(wxSize(0, 0));
    m_panel->Refresh();
}

void ScreenshotEditorFrame::OnColorSelect(wxCommandEvent& event) {
    switch (event.GetId()) {
        case ID_COLOR_YELLOW:
            m_currentColor = *wxYELLOW;
            break;
        case ID_COLOR_BLACK:
            m_currentColor = *wxBLACK;
            break;
        case ID_COLOR_RED:
            m_currentColor = *wxRED;
            break;
        case ID_COLOR_WHITE:
            m_currentColor = *wxWHITE;
            break;
    }
}

// =========================================================
// Implementação dos Métodos de Edição
// =========================================================

void ScreenshotEditorFrame::ApplyCrop() {
    if (m_selectionRect.IsEmpty()) return;

    // Recorta a imagem
    m_currentImage = m_currentImage.GetSubImage(m_selectionRect);

    // Atualiza o bitmap e o tamanho da janela
    m_bitmap = wxBitmap(m_currentImage);
    SetClientSize(m_currentImage.GetSize());

    // Força a repintura
    m_panel->Refresh();
    wxMessageBox("Imagem recortada com sucesso!", "Crop", wxOK);
}

void ScreenshotEditorFrame::ApplyBlur(int radius) {
    if (m_selectionRect.IsEmpty()) return;

    wxImage sourceImage = m_currentImage;
    unsigned char* data = m_currentImage.GetData();
    int width = m_currentImage.GetWidth();
    int height = m_currentImage.GetHeight();

    const unsigned char* src = sourceImage.GetData();

    // Box Blur implementation... (código é idêntico ao original)
    for (int y = m_selectionRect.GetTop(); y <= m_selectionRect.GetBottom(); ++y) {
        for (int x = m_selectionRect.GetLeft(); x <= m_selectionRect.GetRight(); ++x) {

            long sumR = 0, sumG = 0, sumB = 0;
            int count = 0;

            for (int ky = -radius; ky <= radius; ++ky) {
                for (int kx = -radius; kx <= radius; ++kx) {

                    int targetX = x + kx;
                    int targetY = y + ky;

                    if (targetX >= m_selectionRect.GetLeft() &&
                        targetX <= m_selectionRect.GetRight() &&
                        targetY >= m_selectionRect.GetTop() &&
                        targetY <= m_selectionRect.GetBottom() &&
                        targetX >= 0 && targetX < width &&
                        targetY >= 0 && targetY < height) {

                        long index = (static_cast<long>(targetY) * width + targetX) * 3;

                        sumR += src[index];
                        sumG += src[index + 1];
                        sumB += src[index + 2];
                        count++;
                    }
                }
            }

            long index = (static_cast<long>(y) * width + x) * 3;
            if (count > 0) {
                data[index] = (unsigned char)(sumR / count);
                data[index + 1] = (unsigned char)(sumG / count);
                data[index + 2] = (unsigned char)(sumB / count);
            }
        }
    }

    m_bitmap = wxBitmap(m_currentImage);
    m_panel->Refresh();
    wxMessageBox("Desfoque aplicado com sucesso!", "Blur", wxOK);
}

void ScreenshotEditorFrame::ApplyPixelate(int blockSize) {
    if (m_selectionRect.IsEmpty() || blockSize <= 1) return;

    wxImage sourceImage = m_currentImage;
    unsigned char* data = m_currentImage.GetData();
    int width = m_currentImage.GetWidth();
    int height = m_currentImage.GetHeight();

    // Pixelate implementation... (código é idêntico ao original)
    for (int blockY = m_selectionRect.GetTop(); blockY < m_selectionRect.GetBottom(); blockY += blockSize) {
        for (int blockX = m_selectionRect.GetLeft(); blockX < m_selectionRect.GetRight(); blockX += blockSize) {

            long sumR = 0, sumG = 0, sumB = 0;
            int count = 0;

            // 1. Calcular a cor média do bloco
            for (int y = blockY; y < blockY + blockSize && y < m_selectionRect.GetBottom(); ++y) {
                for (int x = blockX; x < blockX + blockSize && x < m_selectionRect.GetRight(); ++x) {

                    long index = (y * width + x) * 3;

                    sumR += sourceImage.GetData()[index];
                    sumG += sourceImage.GetData()[index + 1];
                    sumB += sourceImage.GetData()[index + 2];
                    count++;
                }
            }

            if (count == 0) continue;

            unsigned char avgR = (unsigned char)(sumR / count);
            unsigned char avgG = (unsigned char)(sumG / count);
            unsigned char avgB = (unsigned char)(sumB / count);

            // 2. Preencher todo o bloco com a cor média
            for (int y = blockY; y < blockY + blockSize && y < m_selectionRect.GetBottom(); ++y) {
                for (int x = blockX; x < blockX + blockSize && x < m_selectionRect.GetRight(); ++x) {

                    long index = (y * width + x) * 3;

                    data[index] = avgR;
                    data[index + 1] = avgG;
                    data[index + 2] = avgB;
                }
            }
        }
    }

    m_bitmap = wxBitmap(m_currentImage);
    m_panel->Refresh();
    wxMessageBox("Pixelização aplicada com sucesso!", "Pixelizar", wxOK);
}

void ScreenshotEditorFrame::CopyToClipboard() const {
    if (!m_bitmap.IsOk()) {
        wxMessageBox("Nenhuma imagem disponível para copiar.", "Erro",
                     wxOK | wxICON_ERROR);
        return;
    }

    wxBitmap bitmapToCopy;
    if (!m_selectionRect.IsEmpty()) {
        wxRect imgRect(0, 0, m_currentImage.GetWidth(), m_currentImage.GetHeight());
        wxRect sel = m_selectionRect;
        sel.Intersect(imgRect);

        if (!sel.IsEmpty()) {
            wxImage subImg = m_currentImage.GetSubImage(sel);
            bitmapToCopy = wxBitmap(subImg);
        } else {
            bitmapToCopy = m_bitmap;
        }
    } else {
        bitmapToCopy = m_bitmap;
    }

    if (wxTheClipboard->Open()) {
        wxTheClipboard->Clear();
        const bool ok = wxTheClipboard->SetData(new wxBitmapDataObject(bitmapToCopy));
        wxTheClipboard->Close();

        if (ok) {
            wxMessageBox("Imagem copiada para a Área de Transferência!",
                         "Copiado", wxOK | wxICON_INFORMATION);
        } else {
            wxMessageBox("Falha ao copiar a imagem para a Área de Transferência.",
                         "Erro", wxOK | wxICON_ERROR);
        }
    } else {
        wxMessageBox("Não foi possível acessar a Área de Transferência.",
                     "Erro", wxOK | wxICON_ERROR);
    }
}

void ScreenshotEditorFrame::SaveImage() {

    wxFileDialog saveFileDialog(nullptr, _("Salvar Imagem"), "", "",
                                "PNG files (*.png)|*.png|JPEG files (*.jpg;*.jpeg)|*.jpg;*.jpeg",
                                wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString path = saveFileDialog.GetPath();
    wxString ext = path.AfterLast('.').Lower();
    wxBitmapType type = wxBITMAP_TYPE_PNG;
    if (ext == "jpg" || ext == "jpeg") {
        type = wxBITMAP_TYPE_JPEG;
    }else {

    }

    auto m_current_image = m_currentImage;

    if (!m_selectionRect.IsEmpty()) {
        wxRect imgRect(0, 0, m_current_image.GetWidth(), m_current_image.GetHeight());
        wxRect sel = m_selectionRect;

        sel.Intersect(imgRect);
        if (!sel.IsEmpty()) {
            // wxImage subImg = m_current_image.GetSubImage(sel);
            m_current_image = m_current_image.GetSubImage(sel);
        }
    }

    if (m_current_image.SaveFile(path, type)) {
         wxMessageBox("Imagem salva com sucesso!", "Salvar", wxOK | wxICON_INFORMATION);
    } else {
         wxMessageBox("Erro ao salvar a imagem.", "Erro", wxOK | wxICON_ERROR);
    }
}