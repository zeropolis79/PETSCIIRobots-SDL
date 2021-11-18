#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QImage>
#include <QFile>
#include <QDebug>
#include <cmath>
#include <cstring>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream standardError(stderr);

    // Parse command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("Tiletool");
    parser.addHelpOption();
    parser.addVersionOption();

    QStringList args;
    for (int i = 0; i < argc; i++) {
        args << argv[i];
    }
    QCommandLineOption reverseOption(QStringList() << "r" << "reverse", QCoreApplication::translate("main", "Convert back to 16x16."));
    parser.addOption(reverseOption);
    parser.addPositionalArgument("input", QCoreApplication::translate("main", "The name of the image to be converted."));
    parser.addPositionalArgument("output", QCoreApplication::translate("main", "The basename of the converted image."));
    parser.process(args);

    bool reverse = parser.isSet(reverseOption);

    const QStringList positionalArguments = parser.positionalArguments();

    if (positionalArguments.count() != 2) {
        standardError << QCoreApplication::translate("main", "Invalid arguments") << "\n";
        return 1;
    }

    QImage inputImage(positionalArguments.first());
    if (inputImage.colorCount() == 0) {
        standardError << QCoreApplication::translate("main", "Not a paletted image") << "\n";
        return 1;
    }

    standardError << QCoreApplication::translate("main", "Input image:\n");
    standardError << QCoreApplication::translate("main", "Size ") << inputImage.width() << "x" << inputImage.height() << "\n";
    standardError << QCoreApplication::translate("main", "Colors ") << inputImage.colorCount() << "\n";

    if (reverse) {
        QImage outputImage(24 * 16, 24 * 16, inputImage.format());
        outputImage.setColorCount(inputImage.colorCount());
        outputImage.setColorTable(inputImage.colorTable());
        for (int tile = 0; tile < 256; tile++) {
            int inputY = tile * 24;
            int outputX = (tile % 16) * 24;
            int outputY = (tile / 16) * 24;
            for (int y = 0; y < 24; y++) {
                for (int x = 0; x < 24; x++) {
                    outputImage.setPixel(outputX + x, outputY + y, inputImage.pixelIndex(x, inputY + y));
                }
            }
        }
        outputImage.save(positionalArguments.at(1));

    } else {
        QImage outputImage(32, 24 * 256, inputImage.format());
        outputImage.setColorCount(inputImage.colorCount());
        outputImage.setColorTable(inputImage.colorTable());
        for (int tile = 0; tile < 256; tile++) {
            int inputX = (tile % 16) * 24;
            int inputY = (tile / 16) * 24;
            int outputY = tile * 24;
            for (int y = 0; y < 24; y++) {
                for (int x = 0; x < 24; x++) {
                    outputImage.setPixel(x, outputY + y, inputImage.pixelIndex(inputX + x, inputY + y));
                }
                for (int x = 24; x < 32; x++) {
                    outputImage.setPixel(x, outputY + y, 0);
                }
            }
        }
        outputImage.save(positionalArguments.at(1));
    }

    return 0;
}
