#pragma once
#include <vector>
using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

#include "Mundo2.h" 

namespace WinGame {

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

            if (imagePath != nullptr && System::IO::File::Exists(imagePath))
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
        Image^ imgIdle;
        Image^ imgLeft;
        Image^ imgRight;

        Jugador(int x, int y) :
            Entidad(x, y, 40, 40, 6, "FILES/sprites/player_idle.png")
        {
            up = down = left = right = false;

            try {
                String^ pIdle = "FILES/sprites/player_idle.png";
                String^ pLeft = "FILES/sprites/player_left.png";
                String^ pRight = "FILES/sprites/player_right.png";

                if (System::IO::File::Exists(pIdle)) imgIdle = Image::FromFile(pIdle);
                if (System::IO::File::Exists(pLeft)) imgLeft = Image::FromFile(pLeft);
                if (System::IO::File::Exists(pRight)) imgRight = Image::FromFile(pRight);

                if (imgIdle != nullptr) sprite->Image = imgIdle;
            }
            catch (...) {}
        }

        void actualizar(int maxW, int maxH, Rectangle zona) {
            int dx = 0;
            int dy = 0;

            if (left)  dx -= velocidad;
            if (right) dx += velocidad;
            if (up)    dy -= velocidad;
            if (down)  dy += velocidad;

            int nx = sprite->Left + dx;
            int ny = sprite->Top + dy;

            if (nx < zona.X) nx = zona.X;
            if (nx + sprite->Width > zona.Right) nx = zona.Right - sprite->Width;

            if (ny < zona.Y) ny = zona.Y;
            if (ny + sprite->Height > zona.Bottom) ny = zona.Bottom - sprite->Height;

            sprite->Location = Point(nx, ny);

            if (left && imgLeft != nullptr) sprite->Image = imgLeft;
            else if (right && imgRight != nullptr) sprite->Image = imgRight;
            else if (imgIdle != nullptr) sprite->Image = imgIdle;
        }
    };

    // ==== ENEMIGO ====
    public ref class Enemigo : public Entidad {
    public:
        int dx, dy;
        bool dialogado;
        Point centro;
        double vx, vy;
        double maxSpeed;
        double accel;

        Enemigo(int x, int y, int vel, int dxx, int dyy, String^ img)
            : Entidad(x, y, 40, 40, vel, img)
        {
            dx = dxx; dy = dyy;
            dialogado = false;
            centro = Point(x, y);
            vx = vy = 0.0;
            maxSpeed = (double)vel;
            accel = 0.4;
        }
    };

    // ==== ALIADO ====
    public ref class Aliado : public Entidad {
    public:
        String^ dialogo;
        bool hablado;

        Aliado(int x, int y) : Entidad(x, y, 45, 45, 0, "FILES/sprites/aliado.png") {
            dialogo = "Consejo 1: No toda eficiencia es buena.\nConsejo 2: A veces ir rápido te encierra.\nConsejo 3: Observa antes de actuar.";
            hablado = false;
        }
    };

    // ==== RECURSO ====
    public ref class Recurso : public Entidad {
    public:
        bool usado;
        int eficiencia;

        Recurso(int x, int y, int eficienciaBonus) : Entidad(x, y, 30, 30, 0, "FILES/sprites/recurso.png") {
            usado = false;
            eficiencia = eficienciaBonus;
        }
    };

    // ==== BARRA DE VIDA ====
    public ref class BarraVida {
    public:
        Panel^ fondo;
        Panel^ barra;
        int maxVida;
        int vida;

        BarraVida(int x, int y, int ancho, int alto, int vidaInicial) {
            maxVida = vidaInicial;
            vida = vidaInicial;

            fondo = gcnew Panel();
            fondo->Location = Point(x, y);
            fondo->Size = Drawing::Size(ancho, alto);
            fondo->BackColor = Color::Black;

            barra = gcnew Panel();
            barra->Location = Point(0, 0);
            barra->Size = Drawing::Size(ancho, alto);
            barra->BackColor = Color::LimeGreen;

            fondo->Controls->Add(barra);
        }

        void recibirDaño(int d) {
            vida -= d;
            if (vida < 0) vida = 0;
            actualizarBarra();
        }

        void curar(int c) {
            vida += c;
            if (vida > maxVida) vida = maxVida;
            actualizarBarra();
        }

    private:
        void actualizarBarra() {
            double porcentaje = (double)vida / maxVida;
            barra->Width = (int)(fondo->Width * porcentaje);

            if (porcentaje > 0.6) barra->BackColor = Color::LimeGreen;
            else if (porcentaje > 0.3) barra->BackColor = Color::Orange;
            else barra->BackColor = Color::Red;
        }
    };


    //  FORM JUEGO (MUNDO 1)

    public ref class FormJuego : public System::Windows::Forms::Form
    {
    private:
        Panel^ escena;
        Timer^ tJugador;
        Timer^ tEnemigos;
        Jugador^ jugador;
        List<Enemigo^>^ enemigos;
        Image^ fondo;
        BarraVida^ barraVida;
        int invulnerableTimer;
        Rectangle zonaSegura;

        Rectangle zonaSalida;  

        Aliado^ aliado;
        List<Recurso^>^ recursos;
        String^ objetivoTexto;
        bool objetivoCompletado;
        int conocimiento;

    public:
        FormJuego() {
            // ----- tamaño y ventana -----
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

            zonaSegura = Rectangle(70, 70, 760, 360);

            // =============== ZONA DE SALIDA MUNDO 1 =====================
            zonaSalida = Rectangle(escena->Width - 80, escena->Height - 140, 60, 120);

            // DEBUG: ver visualmente la puerta (puedes borrar luego)
            PictureBox^ puertaDebug = gcnew PictureBox();
            puertaDebug->Location = Point(zonaSalida.X, zonaSalida.Y);
            puertaDebug->Size = Drawing::Size(zonaSalida.Width, zonaSalida.Height);
            puertaDebug->BackColor = Color::FromArgb(120, Color::Blue);
            escena->Controls->Add(puertaDebug);
            puertaDebug->BringToFront();


            int spawnX = (escena->Width / 2) - 15;
            int spawnY = 105;
            jugador = gcnew Jugador(spawnX, spawnY);
            escena->Controls->Add(jugador->sprite);

            barraVida = gcnew BarraVida(20, escena->Bottom + 10, 200, 20, 100);
            this->Controls->Add(barraVida->fondo);

            invulnerableTimer = 0;

            enemigos = gcnew List<Enemigo^>();

            try {
                Enemigo^ e1 = gcnew Enemigo((escena->Width / 2) - 20,
                    (escena->Height / 2) - 20,
                    3, 0, 0,
                    "FILES/sprites/enemigo1.png");

                enemigos->Add(e1);
                escena->Controls->Add(e1->sprite);
            }
            catch (Exception^ ex) {
                MessageBox::Show("Error creando enemigo:\n" + ex->ToString(), "Excepción");
            }

            aliado = gcnew Aliado(escena->Width - 150, 100);
            if (System::IO::File::Exists("FILES/sprites/aliado1.png"))
                aliado->sprite->Image = Image::FromFile("FILES/sprites/aliado1.png");
            escena->Controls->Add(aliado->sprite);

            recursos = gcnew List<Recurso^>();
            recursos->Add(gcnew Recurso(500, 120, 2));
            recursos->Add(gcnew Recurso(250, 300, 1));
            for each (Recurso ^ r in recursos)
                escena->Controls->Add(r->sprite);

            objetivoTexto = "¿Es preferible un mundo totalmente eficiente si sacrifica tu libertad?";
            objetivoCompletado = false;
            conocimiento = 0;

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

        // ==== cargar fondo ====
        void cargarFondo() {
            String^ path = "FILES/backgrounds/mundo1.png";

            if (System::IO::File::Exists(path))
                fondo = Image::FromFile(path);
        }

        void dibujarFondo(Object^ sender, PaintEventArgs^ e) {
            if (fondo != nullptr)
                e->Graphics->DrawImage(fondo, 0, 0, escena->Width, escena->Height);
            else
                e->Graphics->Clear(escena->BackColor);

            e->Graphics->DrawString(objetivoTexto, gcnew System::Drawing::Font("Arial", 10), Brushes::White, PointF(10.0f, 8.0f));
            e->Graphics->DrawString("Puntos de comprensión: " + conocimiento.ToString(), gcnew System::Drawing::Font("Arial", 9), Brushes::White, PointF(10.0f, 24.0f));
        }

        void keyDown(Object^ sender, KeyEventArgs^ e) {
            if (e->KeyCode == Keys::A || e->KeyCode == Keys::Left)  jugador->left = true;
            if (e->KeyCode == Keys::D || e->KeyCode == Keys::Right) jugador->right = true;
            if (e->KeyCode == Keys::W || e->KeyCode == Keys::Up)    jugador->up = true;
            if (e->KeyCode == Keys::S || e->KeyCode == Keys::Down)  jugador->down = true;

            if (e->KeyCode == Keys::E) {
                if (jugador->getBounds().IntersectsWith(aliado->getBounds())) {
                    if (!aliado->hablado) {
                        MessageBox::Show(aliado->dialogo, "Aliado");
                        aliado->hablado = true;
                        ganarConocimiento(1);
                    }
                    else {
                        MessageBox::Show("El aliado ya te dio su punto de vista.", "Aliado");
                    }
                }

                for each (Recurso ^ r in recursos) {
                    if (jugador->getBounds().IntersectsWith(r->getBounds()) && !r->usado) {
                        jugador->velocidad += r->eficiencia;
                        zonaSegura = Rectangle(zonaSegura.X + 30, zonaSegura.Y + 20, Math::Max(100, zonaSegura.Width - 60), Math::Max(80, zonaSegura.Height - 40));
                        r->usado = true;
                        ganarConocimiento(1);
                        MessageBox::Show("Has utilizado un recurso: +eficiencia, -libertad", "Recurso");
                    }
                }
            }
        }

        void keyUp(Object^ sender, KeyEventArgs^ e) {
            if (e->KeyCode == Keys::A || e->KeyCode == Keys::Left)  jugador->left = false;
            if (e->KeyCode == Keys::D || e->KeyCode == Keys::Right) jugador->right = false;
            if (e->KeyCode == Keys::W || e->KeyCode == Keys::Up)    jugador->up = false;
            if (e->KeyCode == Keys::S || e->KeyCode == Keys::Down)  jugador->down = false;
        }


        //  ACTUALIZACIÓN JUGADOR 
        void tickJugador(Object^ sender, EventArgs^ e) {
            jugador->actualizar(escena->Width, escena->Height, zonaSegura);

            if (invulnerableTimer > 0) invulnerableTimer--;

            // CAMBIO DE MUNDO 
            if (jugador->getBounds().IntersectsWith(zonaSalida)) {
                abrirMundo2();
            }
        }

        // ENEMIGOS 
        void tickEnemigos(Object^ sender, EventArgs^ e) {
            for each (Enemigo ^ en in enemigos) {

                if (!en->dialogado) {
                    double px = jugador->sprite->Left + jugador->sprite->Width / 2.0;
                    double py = jugador->sprite->Top + jugador->sprite->Height / 2.0;
                    double ex = en->sprite->Left + en->sprite->Width / 2.0;
                    double ey = en->sprite->Top + en->sprite->Height / 2.0;

                    double dxp = px - ex;
                    double dyp = py - ey;
                    double dist = System::Math::Sqrt(dxp * dxp + dyp * dyp);

                    if (dist < 150.0) {
                        if (dist > 1.0) {
                            double nx = dxp / dist;
                            double ny = dyp / dist;

                            en->vx += nx * en->accel;
                            en->vy += ny * en->accel;

                            double speed = System::Math::Sqrt(en->vx * en->vx + en->vy * en->vy);
                            if (speed > en->maxSpeed) {
                                en->vx = en->vx / speed * en->maxSpeed;
                                en->vy = en->vy / speed * en->maxSpeed;
                            }
                            en->sprite->Left += (int)System::Math::Round(en->vx);
                            en->sprite->Top += (int)System::Math::Round(en->vy);
                        }
                    }
                    else {
                        double cx = en->centro.X + en->sprite->Width / 2.0;
                        double cy = en->centro.Y + en->sprite->Height / 2.0;
                        double rdx = cx - (en->sprite->Left + en->sprite->Width / 2.0);
                        double rdy = cy - (en->sprite->Top + en->sprite->Height / 2.0);
                        double rdist = System::Math::Sqrt(rdx * rdx + rdy * rdy);

                        if (rdist > 1.0) {
                            double rnx = rdx / rdist;
                            double rny = rdy / rdist;

                            en->vx += rnx * 0.2;
                            en->vy += rny * 0.2;
                        }

                        en->vx *= 0.88;
                        en->vy *= 0.88;

                        en->sprite->Left += (int)System::Math::Round(en->vx);
                        en->sprite->Top += (int)System::Math::Round(en->vy);

                        if (System::Math::Abs(rdist) < 2.5) {
                            en->sprite->Location = en->centro;
                            en->vx = en->vy = 0.0;
                        }
                    }

                    if (jugador->getBounds().IntersectsWith(en->getBounds())) {
                        if (invulnerableTimer == 0) {
                            barraVida->recibirDaño(10);
                            invulnerableTimer = 30;
                        }
                    }

                    if (dist < 40.0 && !en->dialogado) {
                        en->dialogado = true;
                        if (tEnemigos != nullptr && tEnemigos->Enabled) tEnemigos->Stop();

                        MessageBox::Show(
                            "Enemigo: ¡Te atrapé! Ahora entenderás por qué este mundo sacrifica libertad por eficiencia...",
                            "Enemigo"
                        );

                        en->sprite->Location = en->centro;
                        en->vx = en->vy = 0.0;

                        if (tEnemigos != nullptr && !tEnemigos->Enabled) tEnemigos->Start();
                    }
                }
                else {
                    en->sprite->Location = en->centro;
                    en->vx = en->vy = 0.0;
                }
            }
        }

        void moverEnemigoEnZona(Enemigo^ en) {
            en->sprite->Left += en->dx * en->velocidad;
            en->sprite->Top += en->dy * en->velocidad;

            Rectangle s = en->getBounds();

            if (s.Left <= zonaSegura.Left || s.Right >= zonaSegura.Right)
                en->dx *= -1;

            if (s.Top <= zonaSegura.Top || s.Bottom >= zonaSegura.Bottom)
                en->dy *= -1;

            if (s.Left < zonaSegura.Left)
                en->sprite->Left = zonaSegura.Left;
            if (s.Right > zonaSegura.Right)
                en->sprite->Left = zonaSegura.Right - en->sprite->Width;
            if (s.Top < zonaSegura.Top)
                en->sprite->Top = zonaSegura.Top;
            if (s.Bottom > zonaSegura.Bottom)
                en->sprite->Top = zonaSegura.Bottom - en->sprite->Height;
        }

        void ganarConocimiento(int puntos) {
            conocimiento += puntos;
            if (conocimiento >= 3 && !objetivoCompletado) {
                objetivoCompletado = true;
                MessageBox::Show("Has comprendido el conflicto: sigue explorando y reflexionando.", "Objetivo cumplido");
            }
        }

        void abrirMundo2() {

            if (tJugador != nullptr) tJugador->Stop();
            if (tEnemigos != nullptr) tEnemigos->Stop();

            try {
                // Obtener estado del jugador actual
                Point playerPos = jugador->sprite->Location;
                Image^ playerImg = jugador->sprite->Image;
                int playerVel = jugador->velocidad;

                // PASAR también las imágenes de animación del Jugador
                Image^ iIdle = jugador->imgIdle;
                Image^ iLeft = jugador->imgLeft;
                Image^ iRight = jugador->imgRight;

                WinGame::FormMundo2^ m2 = gcnew WinGame::FormMundo2(playerPos, playerImg, playerVel, iIdle, iLeft, iRight);
                m2->StartPosition = FormStartPosition::CenterScreen;
                m2->Show();

                this->Close();
            }
            catch (Exception^ ex) {
                MessageBox::Show("No se pudo abrir Mundo 2:\n" + ex->ToString(), "Error");
                if (tJugador != nullptr) tJugador->Start();
                if (tEnemigos != nullptr) tEnemigos->Start();
            }
        }


    };

}
