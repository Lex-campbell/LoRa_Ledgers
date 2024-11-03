#ifndef PAYMENTS_H
#define PAYMENTS_H

#include <Arduino.h>
#include "transaction.h"

Transaction ProcessTransaction(Transaction tx);

#endif // PAYMENTS_H