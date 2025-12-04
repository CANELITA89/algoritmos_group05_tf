// Program.cpp
#include "MenuPrincipal.h"
using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
int main(array<String^>^ args)
{
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    try {
        // Asegúrate de que MenuPrincipal está en el namespace WinGame
        Application::Run(gcnew WinGame::MenuPrincipal());
    }
    catch (Exception^ ex) {
        // Mensaje claro si algo explota dentro del constructor del menú
        MessageBox::Show("Error al iniciar la aplicación:\n" + ex->ToString(), "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }

    return 0;
}
