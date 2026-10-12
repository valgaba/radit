#ifndef SPANISHTRANSLATOR_H
#define SPANISHTRANSLATOR_H

#include <QTranslator>

class SpanishTranslator final : public QTranslator
{
public:
    using QTranslator::QTranslator;
    QString translate(const char *context, const char *sourceText,
                      const char *disambiguation = nullptr,
                      int n = -1) const override;
};

#endif // SPANISHTRANSLATOR_H
