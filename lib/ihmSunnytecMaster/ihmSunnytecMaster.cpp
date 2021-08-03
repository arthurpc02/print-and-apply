/* Projeto IHM - Software desenvolvido para a placa industrial v2.0 comunicando com a IHM - v1.0

Funções IHM Sunnytec Master-Menu - v1.0

*/
#include "ihmSunnytecMaster.h"

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

protocoloIhm liquidC;

// Funções:
ihmSunnytecMaster::ihmSunnytecMaster()
{
}

void ihmSunnytecMaster::imprimirRS485()
{
    liquidC.imprimirImpressoraZebra();
}

void ihmSunnytecMaster::playRS485()
{
    liquidC.funcaoPlayImpressoraZebra();
}

void ihmSunnytecMaster::statusImpressoraRS485()
{
    liquidC.statusImpressoraZebra();
}

Menu* ihmSunnytecMaster::getMenu()
{
    return currentMenu;
}

void ihmSunnytecMaster::display()
{
    if (line1 != oldLine1)
    {
        liquidC.setCursor("0", "0");
        liquidC.print(F("                    "));
        liquidC.setCursor("0", "0");
        liquidC.print(line1);
        oldLine1 = line1;
    }
    if (line2 != oldLine2)
    {
        liquidC.setCursor("1", "0");
        liquidC.print(F("                    "));
        liquidC.setCursor("1", "0");
        liquidC.print(line2);
        oldLine2 = line2;
    }
    if (line3 != oldLine3)
    {
        liquidC.setCursor("2", "0");
        liquidC.print(F("                    "));
        liquidC.setCursor("2", "0");
        liquidC.print(line3);
        oldLine3 = line3;
    }
    if (line4 != oldLine4)
    {
        liquidC.setCursor("3", "0");
        liquidC.print(F("                    "));
        liquidC.setCursor("3", "0");
        liquidC.print(line4);
        oldLine4 = line4;
    }
}

void ihmSunnytecMaster::displayFull()
{
    liquidC.setCursor("0", "0");
    liquidC.print(F("                    "));
    liquidC.setCursor("0", "0");
    liquidC.print(line1);
    oldLine1 = line1;
    liquidC.setCursor("1", "0");
    liquidC.print(F("                    "));
    liquidC.setCursor("1", "0");
    liquidC.print(line2);
    oldLine2 = line2;
    liquidC.setCursor("2", "0");
    liquidC.print(F("                    "));
    liquidC.setCursor("2", "0");
    liquidC.print(line3);
    oldLine3 = line3;
    liquidC.setCursor("3", "0");
    liquidC.print(F("                    "));
    liquidC.setCursor("3", "0");
    liquidC.print(line4);
    oldLine4 = line4;
}

void ihmSunnytecMaster::focus(Menu *menu_f)
{
    selectMenu(menu_f);
}

void ihmSunnytecMaster::updateStatusMonitor(String message, Menu *p_menu)
{
    p_menu->changeMsg(message);
    flag_variavelMudou = true;
}

void ihmSunnytecMaster::signalVariableChange()
{
    flag_variavelMudou = true;
}

void ihmSunnytecMaster::setup()
{
    liquidC.init();
    configDefaultScreen();
    display();
}

void ihmSunnytecMaster::addMenuToIndex(Menu *menu)
{
    menuList[menuCount] = menu;
    menuCount++;
}

void ihmSunnytecMaster::removeMenuFromIndex()
{
    menuCount--;
    menuList[menuCount] = NULL;
}

void ihmSunnytecMaster::configDefaultMsg(String msg)
{
    default_msg = msg;
}

void ihmSunnytecMaster::configDefaultMsg2(String msg)
{
    default_msg2 = msg;
}

void ihmSunnytecMaster::showStatus2msg(String msg) // Muda a quarta linha do display para a mensagem enviada, independente de qual menu está ativo.
{
    status2 = msg;
    flag_status2 = true;
}

void ihmSunnytecMaster::changeMenu(int16_t direction) // Passa para o anterior ou próximo menu, dependendo das posições dos Menus no index.
{
    static int16_t index = 0;
    index += constrain(direction, -1, 1);

    if (index >= (int)menuCount)
    {
        index = 0;
    }
    else if (index < 0)
    {
        index = menuCount - 1;
    }

    selectMenu(menuList[index]);
}

void ihmSunnytecMaster::selectMenu(Menu *menu)
{
    flag_menuChanged = true;
    currentMenu = menu;
}

void ihmSunnytecMaster::eventProcessor_parametros()
{
    if (checkForChangedVariables())
    {
        flag_variavelMudou = false;
        if (currentMenu->getFloat() != 0)
        {
            uint16_t divider = 1;
            for (int i = 0; i < currentMenu->getFloat(); i++)
            {
                divider = divider * 10;
            }
            line3 = String((float)*currentMenu->getVariavel() / divider);
        }
        else
        {
            line3 = String(*currentMenu->getVariavel());
        }
        line3.concat(F(" "));
        line3.concat(currentMenu->getUnidade());
    }
    else if (flag_menuChanged == true)
    {
        flag_menuChanged = false;
        estruturaMenu_parametros();
    }
    else if (flag_status2 == true)
    {
        flag_status2 = false;
        line4 = status2;
    }
}

void ihmSunnytecMaster::eventProcessor_parametros_manu()
{
    if (checkForChangedVariables())
    {
        flag_variavelMudou = false;
        if (currentMenu->getFloat() != 0)
        {
            uint16_t divider = 1;
            for (int i = 0; i < currentMenu->getFloat(); i++)
            {
                divider = divider * 10;
            }
            line3 = String((float)*currentMenu->getVariavel() / divider);
        }
        else
        {
            line3 = String(*currentMenu->getVariavel());
        }
        line3.concat(F(" "));
        line3.concat(currentMenu->getUnidade());
    }
    else if (flag_menuChanged == true)
    {
        flag_menuChanged = false;
        estruturaMenu_parametros_manu();
    }
    else if (flag_status2 == true)
    {
        flag_status2 = false;
        line4 = status2;
    }
}

void ihmSunnytecMaster::eventProcessor_parametros_string()
{
    // rotina que trata os botões acionados
    if (flag_variavelMudou == true)
    {
        flag_variavelMudou = false;
        line3 = String(currentMenu->getMsg());
    }
    else if (flag_menuChanged == true)
    {
        flag_menuChanged = false;
        estruturaMenu_parametros_string();
    }
    else if (flag_status2 == true)
    {
        flag_status2 = false;
        line4 = status2;
    }
}

void ihmSunnytecMaster::eventProcessor_readOnly()
{
    // Rotina que trata os botões acionados
    if (checkForChangedVariables())
    {
        flag_variavelMudou = false;
        line3 = String(*currentMenu->getVariavel());
        line3.concat(F(" "));
        line3.concat(currentMenu->getUnidade());
    }
    else if (flag_menuChanged == true)
    {
        flag_menuChanged = false;
        estruturaMenu_readOnly();
    }
    else if (flag_status2 == true)
    {
        flag_status2 = false;
        line4 = status2;
    }
}

void ihmSunnytecMaster::eventProcessor_monitor()
{
    // rotina que trata os botões acionados
    if (flag_variavelMudou == true)
    {
        flag_variavelMudou = false;
        estruturaMenu_monitor();
    }
    else if (flag_menuChanged == true)
    {
        flag_menuChanged = false;
        estruturaMenu_monitor();
    }
    else if (flag_status2 == true)
    {
        flag_status2 = false;
        line4 = status2;
    }
}

void ihmSunnytecMaster::estruturaMenu_parametros()
{
    if (currentMenu->getProduto() == NULL)
    {
        line1 = F("--------------------");
    }
    else
    {
        line1 = F("Produto: ");
        line1.concat((String)*currentMenu->getProduto());
    }

    line2 = currentMenu->getName();
    line2.concat(F(": "));

    if (currentMenu->getFloat() != 0)
    {
        uint16_t divider = 1;
        for (int i = 0; i < currentMenu->getFloat(); i++)
        {
            divider = divider * 10;
        }
        line3 = String((float)*currentMenu->getVariavel() / divider);
        line3.concat(F(" "));
        line3.concat(currentMenu->getUnidade());

        line4 = F("min:");
        line4.concat(String(currentMenu->getMinimo() / divider));
        line4.concat(F(" max:"));
        line4.concat(String(currentMenu->getMaximo() / divider));
    }
    else
    {
        line3 = String(*currentMenu->getVariavel());
        line3.concat(F(" "));
        line3.concat(currentMenu->getUnidade());

        line4 = F("min: ");
        line4.concat(String(currentMenu->getMinimo()));
        line4.concat(F(" max: "));
        line4.concat(String(currentMenu->getMaximo()));
    }
}

void ihmSunnytecMaster::estruturaMenu_parametros_manu()
{
    if (currentMenu->getProduto() == NULL)
    {
        line1 = F("-----MANUTENCAO-----");
    }
    else
    {
        line1 = F("Produto: ");
        line1.concat((String)*currentMenu->getProduto());
    }

    line2 = currentMenu->getName();
    line2.concat(F(": "));

    if (currentMenu->getFloat() != 0)
    {
        uint16_t divider = 1;
        for (int i = 0; i < currentMenu->getFloat(); i++)
        {
            divider = divider * 10;
        }
        line3 = String((float)*currentMenu->getVariavel() / divider);
        line3.concat(F(" "));
        line3.concat(currentMenu->getUnidade());

        line4 = F("min:");
        line4.concat(String(currentMenu->getMinimo() / divider));
        line4.concat(F(" max:"));
        line4.concat(String(currentMenu->getMaximo() / divider));
    }
    else
    {
        line3 = String(*currentMenu->getVariavel());
        line3.concat(F(" "));
        line3.concat(currentMenu->getUnidade());

        line4 = F("min: ");
        line4.concat(String(currentMenu->getMinimo()));
        line4.concat(F(" max: "));
        line4.concat(String(currentMenu->getMaximo()));
    }
}

void ihmSunnytecMaster::estruturaMenu_parametros_string()
{
     if (currentMenu->getProduto() == NULL)
    {
        line1 = F("-----MANUTENCAO-----");
    }
    else
    {
        line1 = F("Produto: ");
        line1.concat((String)*currentMenu->getProduto());
    }

    line2 = currentMenu->getName();
    line2.concat(F(": "));

    line3 = currentMenu->getMsg();

    line4 = currentMenu->getMsgDefault();
}

void ihmSunnytecMaster::estruturaMenu_readOnly()
{
    line1 = F("--------------------");

    line2 = currentMenu->getName();
    line2.concat(F(": "));

    line3 = String(*currentMenu->getVariavel());
    line3.concat(F(" "));
    line3.concat(currentMenu->getUnidade());

    line4 = F("min: - ");
    line4.concat(F("  max: -"));
}

void ihmSunnytecMaster::estruturaMenu_monitor()
{
    line1 = currentMenu->menu1->getName();
    line1.concat(F(": "));
    line1.concat(*currentMenu->menu1->getVariavel());

    line2 = currentMenu->menu2->getName();
    line2.concat(F(": "));
    line2.concat(*currentMenu->menu2->getVariavel());

    line3 = currentMenu->menu3->getName();
    line3.concat(F(": "));
    line3.concat(*currentMenu->menu3->getVariavel());

    line4 = currentMenu->menu4->getMsg();
}

void ihmSunnytecMaster::configDefaultScreen()
{
    line1 = F(" SUNNYTEC AUTOMACAO");
    line2 = F("    34 3226-2095");
    line3 = default_msg;
    line4 = default_msg2;
}

bool ihmSunnytecMaster::checkForChangedVariables()
{
    static uint32_t var = 256;
    if (currentMenu->getVariavel() != NULL)
    {
        if (var != *currentMenu->getVariavel())
        {
            var = *currentMenu->getVariavel();
            return true;
        }
    }
    return false;
}

// void ihmSunnytecMaster::eventos() // check for events
// {
//     static uint32_t var = 256;
//     if (currentMenu->getVariavel() != NULL)
//     {
//         if (var != *currentMenu->getVariavel())
//         {
//             var = *currentMenu->getVariavel();
//             flag_variavelMudou = true;
//         }
//     }
// }

bool ihmSunnytecMaster::task()
{
    if (millis() - timer_eventos >= intervalo_eventos)
    {
        timer_eventos = millis();
        switch (currentMenu->getTipo())
        {
        case PARAMETRO:
        {
            eventProcessor_parametros();
            break;
        }
        case PARAMETRO_MANU:
        {
            eventProcessor_parametros_manu();
            break;
        }
        case PARAMETRO_STRING:
        {
            // eventos();
            eventProcessor_parametros_string();
            break;
        }
        case READONLY:
        {
            eventProcessor_readOnly();
            break;
        }
        case MONITOR:
        {
            eventProcessor_monitor();
            break;
        }
        }
        display();
    }
    return false;
}