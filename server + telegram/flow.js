// ============================================================================
// ============================================================================
//
//                           ATTENTION: SCRATCH FILE
//
// ============================================================================
// ============================================================================
//
// This file is a scratch pad for development and testing purposes only.
// It contains experimental code, temporary implementations, and work-in-progress.
//
// DO NOT RELY ON ANY CODE OR FUNCTIONALITY IN THIS FILE
//
// The contents of this file are subject to change without notice and may be
// deleted or significantly altered at any time. It is not maintained as part
// of the main codebase and should be ignored for all practical purposes.
//
// If you have stumbled upon this file, please disregard its contents and refer
// to the official, production-ready code in the main project structure.
//
// ============================================================================
// ============================================================================

import {
  createAuthenticatedClient,
  isPendingGrant,
} from "@interledger/open-payments";
import fs from "fs";
import path from "path";
import {
  createClient,
  createIncomingPaymentGrant,
  createQuoteGrant,
  getWalletAddress,
  createOutgoingPaymentGrant,
  createOutgoingPayment,
  continueOutgoingPaymentGrant,
} from "./lib.js";
import { redisGet, redisSet } from "./store.js";

// Load private key for authentication
const privateKeyPath = path.join(process.cwd(), "private.key");
const privateKey = fs.readFileSync(privateKeyPath, "utf8");

// Sender and receiver wallets
const eur = "https://ilp.interledger-test.dev/7a641094"; // Sender (EUR)
const zar = "https://ilp.interledger-test.dev/yolo2"; // Receiver (ZAR)

export const client = await createClient(
  eur,
  privateKey,
  "ab6842e7-0076-4236-8b0c-4c27aa9dc78c"
);

const walletAddress = await getWalletAddress(client, eur);
console.log("Sender Wallet Address:", walletAddress);

// ==============================

const grant_incomingPayment = await createIncomingPaymentGrant(client, eur);
const grant_quote = await createQuoteGrant(client, eur);
const quote_token = grant_quote.access_token.value;

// let payment_token = "F3B02DF7099FFFDC2EF1";
if (1 == 2) {
  const grant_outgoingPayment = await createOutgoingPaymentGrant(
    client,
    eur,
    { assetCode: "EUR", assetScale: 2, value: "1000" },
    // { assetCode: "ZAR", assetScale: 2, value: "200" },
    "123"
  );
  if (isPendingGrant(grant_incomingPayment)) {
    throw new Error("Expected non-interactive grant");
  }
  console.log(
    "OUTGOING_PAYMENT_ACCESS_TOKEN =",
    grant_outgoingPayment.continue.access_token.value
  );
  console.log(
    "OUTGOING_PAYMENT_ACCESS_TOKEN_MANAGE_URL = ",
    grant_outgoingPayment.interact.redirect
  );
  payment_token = grant_outgoingPayment.continue.access_token.value;

  const continueGrant = await continueOutgoingPaymentGrant(
    client,
    payment_token,
    grant_outgoingPayment.continue.uri,
    grant_outgoingPayment.interact.finish
  );
  console.log("CONTINUE_GRANT =", continueGrant);
}

// ==============================

const incomingPayment = await client.incomingPayment.create(
  {
    url: walletAddress.resourceServer,
    accessToken: grant_incomingPayment.access_token.value,
  },
  {
    walletAddress: eur,
    incomingAmount: {
      value: "2",
      assetCode: "EUR",
      assetScale: 2,
    },
    expiresAt: new Date(Date.now() + 60_000 * 10).toISOString(),
  }
);

console.log("Incoming Payment Response:", incomingPayment);

// ==============================

// Create a quote for the incoming payment
const quote = await client.quote.create(
  {
    url: walletAddress.resourceServer,
    accessToken: quote_token,
  },
  {
    method: "ilp",
    walletAddress: eur,
    receiver: incomingPayment.id,
  }
);

console.log("Quote Response:", quote);

// ==============================

const outgoingPayment = await createOutgoingPayment(
  client,
  await redisGet("EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN"),
  walletAddress,
  quote.id
);

console.log("Outgoing Payment Response:", outgoingPayment);

const rotatedToken = await rotateAccessToken(
  client,
  redisGet("EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN_MANAGE_URL"),
  redisGet("EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN")
);

console.log("Rotated Token Response:", rotatedToken);
