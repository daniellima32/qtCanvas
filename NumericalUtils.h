#ifndef MATHOPERATIONS_H
#define MATHOPERATIONS_H

#define nearZero 0.000001

class NumericalUtils
{
public:
    NumericalUtils();

    /**
        * Compara 2 doubles e retorna true caso eles sejam numericamente iguais, considerando um certo epsilon
        * @param x Primeiro número a ser comparado
        * @param y Segundo número a ser comparado
        * @param epsilon Precisão a ser condiserada na comparação. Quanto menor for o valor de epsilon,
        *	mais próximos os números deverão ser para serem considerados iguais.
        * @return
        *	true, caso os números sejam iguais.
        *	false, cc.
        */
    static bool compareDoubles(const double &x, const double &y, const double &epsilon = 0.000001);
};

#endif // MATHOPERATIONS_H
