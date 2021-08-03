#ifndef IHMSUNNYTECMASTER_V1_H
#define IHMSUNNYTECMASTER_V1_H

/* Projeto IHM - Software desenvolvido para a placa industrial v2.0 comunicando com a IHM - v1.0

Biblioteca ihmSunnytecMaster - v1.0

Informações sobre os comandos:
Print: 28
Clear: 29
SetCursor: 31

Para a utilização da impressora zebra utilizar a função imprimirZebra() e statusZebra()
Tamanho máximo de informações 20 caractéres (LCD: 20x4)
*/

#include <protocoloIhm.h>
#include <Menu.h>

class ihmSunnytecMaster
{
public:
    ihmSunnytecMaster();

// Funções:
    void imprimirRS485();

    void playRS485();

    void statusImpressoraRS485();

    Menu *getMenu();

    void display();

    void displayFull();

    void focus(Menu *);

    void updateStatusMonitor(String, Menu *);

    void signalVariableChange();

    void setup();

    bool task();

    void addMenuToIndex(Menu *);

    void removeMenuFromIndex();

    void configDefaultMsg(String);

    void configDefaultMsg2(String);

    void showStatus2msg(String);

#define NEXT 1
#define PREVIOUS -1

    void changeMenu(int16_t);

    void selectMenu(Menu *);

    void eventProcessor_parametros();

    void eventProcessor_parametros_manu();

    void eventProcessor_parametros_string();

    void eventProcessor_readOnly();

    void eventProcessor_monitor();

    void estruturaMenu_parametros();

    void estruturaMenu_parametros_manu();

    void estruturaMenu_parametros_string();

    void estruturaMenu_readOnly();

    void estruturaMenu_monitor();

    void configDefaultScreen();

    bool checkForChangedVariables();

    void eventos();
// Funções:

private:
    Menu *menuList[25] = {NULL, NULL, NULL, NULL, NULL,
                          NULL, NULL, NULL, NULL, NULL,
                          NULL, NULL, NULL, NULL, NULL,
                          NULL, NULL, NULL, NULL, NULL,
                          NULL, NULL, NULL, NULL, NULL};
    // Menu default_menu = Menu("def_menu", STATUS_TRACKER, "def");
    Menu *currentMenu = NULL;
    // Menu Menu_status = Menu("Status", STATUS_TRACKER, "ok");

    unsigned long timer_eventos = 0;
    uint16_t intervalo_eventos = 50; // ms

    // variáveis ihm:
    String line1 = "z";
    String line2 = "z";
    String line3 = "z";
    String line4 = "z";

    String oldLine1 = "z";
    String oldLine2 = "z";
    String oldLine3 = "z";
    String oldLine4 = "z";

    String status2 = " ";

    enum Setas
    {
        nenhuma,
        linha1,
        linha2,
        linha3,
        linha4
    } setas;

    String default_msg = "                   ";
    String default_msg2 = "MAQUINAS EM GERAIS";
    uint16_t menuCount = 0;

    bool flag_menuChanged = false;
    bool flag_variavelMudou = false;
    bool flag_status2 = false;
};

bool task();

#endif