#include <iostream>
#include <vector>
#include <SFML/Audio.hpp>

const int STEREO = 2;
const int MONO = 1;
const int SAMPLE_RATE = 44100;
const int NORM = 32767;
using namespace std;


vector<sf::Int16> generateSine(int freq, int sampleRate, double duration) {
    int sampleCount = sampleRate*duration;
    vector<sf::Int16> res(sampleCount);
    for (int i = 0; i < res.size(); ++i) {
        double x = i*1.0 / sampleRate;
        double y = sin(freq*x * 2 * M_PI);
        res[i] = y * NORM;
    }
    return res;
}

vector<sf::Int16> modulate(const vector<sf::Int16>& data, double base, double exp, int sampleRate) {
    int length;
    {
        double x = (data.size()-1) / sampleRate;
        double t = log(x*log(exp)/base+1) / log(exp);
        int sampleT = t * sampleRate;
        length = sampleT;
    }
    vector<sf::Int16> res(length, 0);
    for (int i = 0; i < res.size(); ++i) {
        double t = i*1.0 / sampleRate;
        double x = base*(pow(exp, t) - 1) / log(exp);
        int sampleX = x * sampleRate;
        if (sampleX < data.size() && sampleX >= 0) {
            res[i] = data[sampleX];
        }
        //cout << i << ": " << x << endl;
    }
    return res;
}

vector<sf::Int16> modulate(int sampleCount, double base, double exp, int sampleRate) {
    int length = sampleCount;
    vector<sf::Int16> res(length, 0);
    for (int i = 0; i < res.size(); ++i) {
        double t = i*1.0 / sampleRate;
        double x = base*(pow(exp, t) - 1) / log(exp);
        double sampleX = x;
//        if (sampleX < data.size() && sampleX >= 0) {
//            res[i] = data[sampleX];
//        }
        res[i] = sin(2*M_PI*sampleX) * NORM;
        //cout << i << ": " << x << endl;
    }
    return res;
}

vector<sf::Int16> amplify(const vector<sf::Int16>& data, double amp) {
    vector<sf::Int16> res(data.size());
    std::transform(data.begin(), data.end(), res.begin(), [amp] (sf::Int16 x) {
        return x*amp;
    });
    return res;
}

void add(vector<sf::Int16>& in, const vector<sf::Int16>& toAdd, int posIn, int posToAdd, int count) {
    std::transform(in.begin()+posIn, in.begin()+posIn+count, toAdd.begin()+posToAdd, in.begin()+posIn,
                   [] (sf::Int16 x, sf::Int16 y) {
        return x+y;
    });
}

vector<sf::Int16> shepherd(int chans, double toneRatio, double chanRatio, double minFreq, double maxFreq, int sampleRate, double ampl) {
    bool isReverse = toneRatio < 1;
    if (isReverse) {
        toneRatio = 1 / toneRatio;
    }
    int sampleCount = log(maxFreq*1.0 / minFreq) / log(toneRatio) * sampleRate;
    vector<sf::Int16> primary = amplify(modulate(sampleCount, minFreq, toneRatio, sampleRate), ampl);
    vector<sf::Int16> res = primary;
    double curPos = 0;
    double curFreq = minFreq;
    int loopCount = 1;
    while (curPos < sampleCount && loopCount < chans) {
//        if (loopCount >= 2) {
//            break;
//        }
        loopCount++;
        curFreq = curFreq * chanRatio;
        curPos = log(curFreq / minFreq) / log(toneRatio) * sampleRate;
        if (curPos >= sampleCount) {
            break;
        }
        cout << "inpos, outpos, count, ressize " << curPos << " " << 0 << " " << res.size()-curPos << " " << res.size() << endl;
        add(res, primary, curPos, 0, res.size()-curPos);
        add(res, primary, 0, primary.size()-curPos, curPos);
    }
    if (isReverse) {
        std::reverse(res.begin(), res.end());
    }
    return res;
}

int main() {
    sf::SoundBuffer buf;
    //vector<sf::Int16> data = generateSine(10, SAMPLE_RATE, 350.0);
    //data = modulate(data, 44, 0.9, SAMPLE_RATE);
    vector<sf::Int16> data = modulate(SAMPLE_RATE*20, 440, 1.11, SAMPLE_RATE);
    //vector<sf::Int16> data = shepherd(10, 1.25, 2, 20, 20480, SAMPLE_RATE, 0.09);
    buf.loadFromSamples(data.data(), data.size()/MONO, MONO, SAMPLE_RATE);
    buf.saveToFile("Files/res.wav");
    return 0;
}
