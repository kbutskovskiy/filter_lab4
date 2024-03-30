#include <iostream>
#include <vector>


namespace
{
    //КИХ коэффициенты моего варианта (не квантованные)
    std::vector<double> kCoefficients = {
            0.000324762308952612278097754616723591425,
            -0.016001996984935276668871040328667731956,
            -0.027425294846693794303948976676110760309,
            -0.02182123534117955815370137884201540146,
            0.020360857916758741403828736338255112059,
            0.0994978461087307891608233489932899829,
            0.189790697324172752402660080406349152327,
            0.25034194763022987739731206602300517261,
            0.25034194763022987739731206602300517261,
            0.189790697324172752402660080406349152327,
            0.0994978461087307891608233489932899829,
            0.020360857916758741403828736338255112059,
            -0.02182123534117955815370137884201540146,
            -0.027425294846693794303948976676110760309,
            -0.016001996984935276668871040328667731956,
            0.000324762308952612278097754616723591425
    };


    constexpr int kOrder = 15;        // order -- кол-во коэффициентов фильтра
    constexpr int kTime = 15;         // simulation time время симуляции, указывающее на количество обрабатываемых сэмплов
    constexpr int kMaximum = 32768;   // maximum in short 2^15
    //значения, используемые для симуляции ступенчатого и импульсного входных сигналов
    constexpr float kStep = 0.8; //амплитуда ступенчатого сигнала
    constexpr float kImpulse = 0.999; // maximum of 1 - 2^(-order)
} // namespace


class Filter
{
public:
    Filter(int order, int simulationTime, const std::vector<double>& coefficients)
    {
        m_frequencyOrder = order; // порядок фильтра -1 = kOrder
        m_simulationTime = simulationTime; // время симуляции = kTime
        m_coefficients = coefficients; // коэффициенты фильтра = coeffs
    }


    void impulse(float value)
    {
        /**
         * Создаем импульсный сигнал. 0.8 от
         */
        m_samples.resize(m_simulationTime, 0);
        m_samples[0] = value;
        filter();
    }


    void step(float value)
    {
        m_samples.resize(m_simulationTime, value); // изменяет размер вектора m_samples до значения указанного в m_simulationTime
        /*
         * Если новый размер больше текущего, то в вектор добавляются новые элементы, инициализированные значением,
         * указанным вторым аргументом метода, который в данном случае равен value. Если новый размер меньше текущего,
         * то лишние элементы удаляются.
         * */
        filter();
    }


    void output()
    {
        for (auto& element : m_result)
        {
            std::cout << element << "\n";
        }
    }


private:
    int m_frequencyOrder;
    int m_simulationTime;
    std::vector<double> m_coefficients;
    std::vector<float> m_samples;

    std::vector<float> m_result;


    std::vector<short> quantizeCoefficients()
    {
        std::vector<short> quantizedCoefficients(m_frequencyOrder + 1); // коэффициентов на 1 больше, чем порядок

        // умножаем число от 0 до 1 на 2^15 и обрезаем до 16 битов -- переводим в вид 1.15
        for (int i = 0; i < m_frequencyOrder + 1; ++i)
        {
            quantizedCoefficients[i] = (short)(m_coefficients[i] * kMaximum);
        }

        return quantizedCoefficients;
    }


public:
    void clear() {
        m_result.clear();
        m_samples.clear();
    }

private:


    std::vector<short> quantizeSamples()
    {
        std::vector<short> quantizedSamples(m_simulationTime); // кол-во отсчетов равно 15

        for (int i = 0; i < m_simulationTime; ++i)
        {
            quantizedSamples[i] = (short)(m_samples[i] * kMaximum);
        }


        return quantizedSamples;
    }

    void convertResults(std::vector<short>& filteringResult)
    {
        for (int i = 0; i < m_simulationTime; ++i)
        {
            m_result.push_back((float)filteringResult[i] / kMaximum);
        }
    }


    void filter()
    {
        int sum;
        std::vector<short> filteringResult; //результат после фильтрации


        auto quantizedCoefficients = quantizeCoefficients(); //квантованные коэффициенты фильтра
        auto quantizedSamples = quantizeSamples(); //квантируем отсчеты -- входной сигнал
        // имеем quantizedCoefficients -- коэффициенты фильтра , их 16. quantizedSamples -- 16 отсчетов
        for (int i = 0; i < m_simulationTime; ++i) {
            sum = 0;
            for (int j = 0; j < m_frequencyOrder; ++j)
            {
                if (i - j >= 0)
                {
                    sum += quantizedSamples[i - j] * quantizedCoefficients[j];
                }
            }
            filteringResult.push_back((short)(sum / kMaximum));
        }


        convertResults(filteringResult);
    }
};


int main()
{
    Filter FIR(kOrder, kTime, kCoefficients);
    FIR.step(kStep);
    std::cout << "step: \n";
    FIR.output();

    FIR.clear();

    std::cout << "\n"
              << "impulse: \n";
    FIR.impulse(kImpulse);
    FIR.output();


    return 0;
}