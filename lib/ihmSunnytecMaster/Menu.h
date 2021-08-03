#ifndef MENU_SUNNYTEC_V1_H
#define MENU_SUNNYTEC_V1_H

/* Projeto IHM - Software desenvolvido para a placa industrial v2.0

Biblioteca Menu IHM - v1.0

*/

enum TipoMenu
{
    PARAMETRO,
    PARAMETRO_MANU,
    PARAMETRO_STRING,
    MONITOR,
    READONLY,
};

enum MaisOuMenos
{
    MENOS = -1,
    NADA = 0,
    MAIS = 1,
};

class Menu
{
public:
    Menu(String par_name = "def",
         TipoMenu par_tipo = READONLY,
         int32_t *par_variavel = NULL,
         String par_unidade = " ",
         uint16_t par_acrescimo = 1,
         uint32_t par_minimo = 0,
         uint32_t par_maximo = 9999,
         int32_t *par_produto = NULL,
         uint16_t par_float = 0)
    {
        name = par_name;
        tipo = par_tipo;
        p_variavel = par_variavel;
        unidade = par_unidade;
        acrescimo = par_acrescimo;
        minimo = par_minimo;
        maximo = par_maximo;
        produto = par_produto;
        _float = par_float;
    }

    Menu(String par_name = "def", TipoMenu par_tipo = PARAMETRO_STRING, String par_msg = " ", int32_t *par_produto = NULL)
    {
        name = par_name;
        tipo = par_tipo;
        messageDefault = par_msg;
        produto = par_produto;
    }

    void changeMsg(String msg)
    {
        message = msg;
    }

    void addVar(MaisOuMenos maisOuMenos) // Incrementa ou decrementa a variável associada ao menu atual.
    {
        if (tipo != READONLY)
        {
            maisOuMenos = static_cast<MaisOuMenos>(constrain(maisOuMenos, -1, 1));
            if (*p_variavel == maximo && maisOuMenos == 1)
            {
                *p_variavel = minimo;
            }
            else if (*p_variavel == minimo && maisOuMenos == -1)
            {
                *p_variavel = maximo;
            }
            else
            {
                *p_variavel += maisOuMenos * ((int)acrescimo);
                *p_variavel = constrain(*p_variavel, minimo, maximo);
            }
        }
    }

    String getName()
    {
        return name;
    }

    int32_t *getVariavel()
    {
        return p_variavel;
    }

    String getUnidade()
    {
        return unidade;
    }

    uint16_t getAcrescimo()
    {
        return acrescimo;
    }

    uint16_t getMinimo()
    {
        return minimo;
    }

    uint32_t getMaximo()
    {
        return maximo;
    }

    uint16_t getFloat()
    {
        return _float;
    }

    TipoMenu getTipo()
    {
        return tipo;
    }

    String getMsg()
    {
        return message;
    }

    String getMsgDefault()
    {
        return messageDefault;
    }

    void setMonitorMenus(Menu *par_menu1 = NULL, Menu *par_menu2 = NULL, Menu *par_menu3 = NULL, Menu *par_menu4 = NULL)
    {
        menu1 = par_menu1;
        menu2 = par_menu2;
        menu3 = par_menu3;
        menu4 = par_menu4;
    }

    int32_t *getProduto()
    {
        return produto;
    }

    Menu *menu1 = NULL;
    Menu *menu2 = NULL;
    Menu *menu3 = NULL;
    Menu *menu4 = NULL;

private:
    String name = "def";
    int32_t *p_variavel = NULL;
    TipoMenu tipo = READONLY;
    uint16_t type = 0;
    String unidade = " ";
    uint16_t acrescimo = 1;
    uint16_t minimo = 0;
    uint32_t maximo = 9999;
    String message = " ";
    String messageDefault = " ";
    int32_t *produto = NULL;
    uint16_t _float = 0; // Quando essa variável for true, o parâmetro apresenta uma casa decimal.
};

#endif