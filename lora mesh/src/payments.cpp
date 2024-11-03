#include <Arduino.h>
#include "message.h"
#include "payments.h"
#include "utils.h"


Transaction ProcessTransaction(Transaction tx) {
    // Simulate payment processing
    delay(3000); // Simulate some processing time
    
    // Randomly succeed or fail the transaction
    if (random(100) < 80) { // 80% success rate
        tx.state = Transaction::STATE_SUCCESS;
    } else {
        tx.state = Transaction::STATE_FAILED;
    }

    SendTelegram(tx.humanString());
    
    return tx;
}