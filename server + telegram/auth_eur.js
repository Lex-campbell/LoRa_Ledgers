import fs from "fs";
import path from "path";
import {
  createClient,
  getWalletAddress,
  createOutgoingPaymentGrant,
  continueOutgoingPaymentGrant,
} from "./lib.js";
import { redisSet, redisGet } from "./store.js";

// Load private key for authentication
const privateKeyPath = path.join(process.cwd(), "private.key");
const privateKey = fs.readFileSync(privateKeyPath, "utf8");

// Sender and receiver wallets
const eur = "https://ilp.interledger-test.dev/7a641094"; // Sender (EUR)
const zar = "https://ilp.interledger-test.dev/yolo2"; // Receiver (ZAR)

const client = await createClient(
  eur,
  privateKey,
  "ab6842e7-0076-4236-8b0c-4c27aa9dc78c"
);

const walletAddress = await getWalletAddress(client, eur);
console.log("Sender Wallet Address:", walletAddress);

// ==============================

if (1 == 1) {
  const grant_outgoingPayment = await createOutgoingPaymentGrant(
    client,
    eur,
    { assetCode: "EUR", assetScale: 2, value: "1000" },
    // { assetCode: "ZAR", assetScale: 2, value: "200" },
    "123"
  );
  await redisSet(
    "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN",
    grant_outgoingPayment.continue.access_token.value
  );
  await redisSet(
    "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN_MANAGE_URL",
    grant_outgoingPayment.interact.redirect
  );
  await redisSet(
    "EUR_OUTGOING_PAYMENT_GRANT_CONTINUE_URI",
    grant_outgoingPayment.continue.uri
  );
}

// ==============================
else {
  // http://localhost:3344/?hash=GaIuRR1a5S3EZJppsPwByQELqblXcx4f6aSAbggXrzs%3D&interact_ref=64b9a354-0a5d-44d0-b2ed-204d94808aa6
  const payment_token = redisGet("EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN");
  const continue_uri = redisGet("EUR_OUTGOING_PAYMENT_GRANT_CONTINUE_URI");
  const interaction_ref = "64b9a354-0a5d-44d0-b2ed-204d94808aa6";

  const continueGrant = await continueOutgoingPaymentGrant(
    client,
    payment_token,
    "https://auth.interledger-test.dev/continue/" + continue_uri,
    interaction_ref
  );
  console.log("CONTINUE_GRANT =", continueGrant);
  if (continueGrant && continueGrant.access_token) {
    console.log("Access Token Value:", continueGrant.access_token.value);
    redisSet(
      "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN",
      continueGrant.access_token.value
    );
    console.log("Expires In:", continueGrant.access_token.expires_in);
  } else {
    console.log(
      "Unable to retrieve access token information from continueGrant"
    );
  }
}
