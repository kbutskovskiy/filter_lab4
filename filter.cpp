#include <iostream>
#include <vector>


namespace
{
    std::vector<double> kCoefficients = {0.002388010102414,  -0.000912612192452, -0.01190975140161,  -0.02494092458485,
                                         -0.008321958102004, 0.0689537917072,    0.1901587549693,    0.284584689502,
                                         0.284584689502,     0.1901587549693,    0.0689537917072,    -0.008321958102004,
                                         -0.02494092458485,  -0.01190975140161,  -0.000912612192452, 0.002388010102414};


    constexpr int kOrder = 15;        // order
    constexpr int kTime = 15;         // simulation time
    constexpr int kMaximum = 32768;   // maximum in short
    constexpr float kStep = 0.8;
    constexpr float kImpulse = 0.999; // maximum of 1 - 2^(-order)
} // namespace


class Filter
{
public:
    Filter(int order, int simulationTime, const std::vector<double>& coefficients)
    {
        m_frequencyOrder = order;
        m_simulationTime = simulationTime;
        m_coefficients = coefficients;
    }


    void impulse(float value)
    {
        m_samples.resize(m_simulationTime, 0);
        m_samples[0] = value;


        filter();
    }


    void step(float value)
    {
        m_samples.resize(m_simulationTime, value);


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
        std::vector<short> quantizedCoefficients(m_frequencyOrder + 1);


        for (int i = 0; i < m_frequencyOrder + 1; ++i)
        {
            quantizedCoefficients[i] = (short)(m_coefficients[i] * kMaximum);
        }


        return quantizedCoefficients;
    }


    std::vector<short> quantizeSamples()
    {
        std::vector<short> quantizedSamples(m_simulationTime);


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
        std::vector<short> filteringResult;


        auto quantizedCoefficients = quantizeCoefficients();
        auto quantizedSamples = quantizeSamples();
        for (int i = 0; i < m_simulationTime; ++i)
        {
            sum = 0;
            for (int j = 0; j < m_frequencyOrder; j++)
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


    std::cout << "\n"
              << "impulse: \n";
    FIR.impulse(kImpulse);
    FIR.output();


    return 0;
}