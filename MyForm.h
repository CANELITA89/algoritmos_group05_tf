// FormJuego.h
#pragma once
#include <vector>

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

namespace WinGame {

    // ==== ENTIDAD BASE ====
    public ref class Entidad {
    public:
        PictureBox^ sprite;
        int velocidad;

        Entidad(int x, int y, int w, int h, int vel, String^ imagePath) {
            velocidad = vel;
            sprite = gcnew PictureBox();
            sprite->Location = Point(x, y);
            sprite->Size = Drawing::Size(w, h);
            sprite->SizeMode = PictureBoxSizeMode::StretchImage;
            sprite->BackColor = Color::Transparent;

            if (System::IO::File::Exists(imagePath))
                sprite->Image = Image::FromFile(imagePath);
        }

        virtual void mover(int dx, int dy) {
            sprite->Left += dx;
            sprite->Top += dy;
        }

        Rectangle getBounds() {
            return Rectangle(sprite->Location, sprite->Size);
        }
    };

    // ==== JUGADOR ====
    public ref class Jugador : public Entidad {
    public:
        bool up, down, left, right;

        Jugador(int x, int y) :
            Entidad(x, y, 40, 40, 6, "FILES/sprites/player.png")
        {
            up = down = left = right = false;
        }

        void actualizar(int maxW, int maxH) {
            int dx = 0, dy = 0;

            if (left)  dx -= velocidad;
            if (right) dx += velocidad;
            if (up)    dy -= velocidad;
            if (down)  dy += velocidad;

            int nx = sprite->Left + dx;
            int ny = sprite->Top + dy;

            if (nx >= 0 && nx + sprite->Width <= maxW)
                sprite->Left = nx;
            if (ny >= 0 && ny + sprite->Height <= maxH)
                sprite->Top = ny;
        }
    };

    // ==== ENEMIGO ====
    public ref class Enemigo : public Entidad {
    public:
        int dx, dy;

        Enemigo(int x, int y, int vel, int dxx, int dyy, String^ img) :
            Entidad(x, y, 40, 40, vel, img)
        {
            dx = dxx;
            dy = dyy;
        }

        void moverAuto(int maxW, int maxH) {

            sprite->Left += dx * velocidad;
            sprite->Top += dy * velocidad;

            if (sprite->Left <= 0 || sprite->Right >= maxW)
                dx = -dx;

            if (sprite->Top <= 0 || sprite->Bottom >= maxH)
                dy = -dy;
        }
    };

    // ==== FORM PRINCIPAL ====
    public ref class FormJuego : public System::Windows::Forms::Form
    {
    private:
        Panel^ escena;
        Timer^ tJugador;
        Timer^ tEnemigos;

        Jugador^ jugador;
        List<Enemigo^>^ enemigos;
        Image^ fondo;

    public:
        FormJuego() {
            this->Width = 960;
            this->Height = 600;
            this->Text = "Proyecto - IA vs Pensamiento Crítico";

            escena = gcnew Panel();
            escena->Location = Point(20, 20);
            escena->Size = Drawing::Size(900, 500);
            escena->BackColor = Color::Black;
            escena->Paint += gcnew PaintEventHandler(this, &FormJuego::dibujarFondo);
            this->Controls->Add(escena);

            cargarFondo();

            jugador = gcnew Jugador(200, 200);
            escena->Controls->Add(jugador->sprite);

            enemigos = gcnew List<Enemigo^>();
            enemigos->Add(gcnew Enemigo(100, 80, 3, 1, 0, "FILES/sprites/enemigo1.png"));
            enemigos->Add(gcnew Enemigo(300, 200, 4, -1, 1, "FILES/sprites/enemigo2.png"));
            enemigos->Add(gcnew Enemigo(600, 300, 3, 1, -1, "FILES/sprites/enemigo1.png"));

            for each (Enemigo ^ e in enemigos)
                escena->Controls->Add(e->sprite);

            tJugador = gcnew Timer();
            tJugador->Interval = 20;
            tJugador->Tick += gcnew EventHandler(this, &FormJuego::tickJugador);
            tJugador->Start();

            tEnemigos = gcnew Timer();
            tEnemigos->Interval = 120;
            tEnemigos->Tick += gcnew EventHandler(this, &FormJuego::tickEnemigos);
            tEnemigos->Start();

            this->KeyPreview = true;
            this->KeyDown += gcnew KeyEventHandler(this, &FormJuego::keyDown);
            this->KeyUp += gcnew KeyEventHandler(this, &FormJuego::keyUp);
        }

        void cargarFondo() {
            String^ path = "FILES/backgrounds/mundo1.png";

            if (System::IO::File::Exists(path))
                fondo = Image::FromFile(path);
        }

        void dibujarFondo(Object^ sender, PaintEventArgs^ e) {
            if (fondo != nullptr)
                e->Graphics->DrawImage(fondo, 0, 0, escena->Width, escena->Height);
        }

        void keyDown(Object^ sender, KeyEventArgs^ e) {
            if (e->KeyCode == Keys::A) jugador->left = true;
            if (e->KeyCode == Keys::D) jugador->right = true;
            if (e->KeyCode == Keys::W) jugador->up = true;
            if (e->KeyCode == Keys::S) jugador->down = true;
        }

        void keyUp(Object^ sender, KeyEventArgs^ e) {
            if (e->KeyCode == Keys::A) jugador->left = false;
            if (e->KeyCode == Keys::D) jugador->right = false;
            if (e->KeyCode == Keys::W) jugador->up = false;
            if (e->KeyCode == Keys::S) jugador->down = false;
        }

        void tickJugador(Object^ sender, EventArgs^ e) {
            jugador->actualizar(escena->Width, escena->Height);
        }

        void tickEnemigos(Object^ sender, EventArgs^ e) {
            for each (Enemigo ^ en in enemigos)
                en->moverAuto(escena->Width, escena->Height);
        }
    };
}
