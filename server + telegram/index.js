// ============================================================================
//                       Open Payments Express Server
// ============================================================================
//
// This is the core Express server that handles network requests and interfaces
// with the first layer of libraries, which in turn interact with the
// Interledger Protocol (ILP).
//
// Key features:
// - Handles incoming HTTP requests for Open Payments operations
// - Interfaces with library functions for ILP interactions
// - Manages authentication and authorization flows
// - Processes incoming and outgoing payments
// - Handles quote creation and management
// - Implements token rotation and management
//
// This server acts as the primary interface between client applications
// and the underlying Open Payments / ILP infrastructure, providing a
// RESTful API for financial operations in a web-based environment.
//
// ============================================================================

import express from "express";
import { redisSet, redisGet } from "./store.js";
import {
  getWalletAddress,
  continueOutgoingPaymentGrant,
  createOutgoingPaymentGrant,
  createIncomingPaymentGrant,
  createOutgoingPayment,
  createQuoteGrant,
  rotateAccessToken,
  createClient,
  eur,
  privateKey,
} from "./lib.js";
const app = express();
const port = 3344;

const client = await createClient(
  eur,
  privateKey,
  "ab6842e7-0076-4236-8b0c-4c27aa9dc78c"
);

// Middleware to parse JSON bodies
app.use(express.json());

// This route handles the root endpoint ("/") and processes incoming interaction references
// It stores the interaction reference, continues the outgoing payment grant,
// and updates the access token information in the Redis store
app.get("/", async (req, res) => {
  const { interact_ref } = req.query;
  if (interact_ref) {
    await redisSet("INTERACTION_REFERENCE", interact_ref);
    console.log("Interaction reference stored:", interact_ref);
  } else {
    res.send("Welcome to the Open Payments API server");
  }

  const payment_token = await redisGet(
    "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN"
  );
  const continue_uri = await redisGet(
    "EUR_OUTGOING_PAYMENT_GRANT_CONTINUE_URI"
  );

  //CONTINUE GRANT

  const continueGrant = await continueOutgoingPaymentGrant(
    client,
    payment_token,
    continue_uri,
    interact_ref
  );
  console.log("CONTINUE_GRANT =", continueGrant);
  if (continueGrant && continueGrant.access_token) {
    console.log("Access Token Value:", continueGrant.access_token.value);
    await redisSet(
      "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN",
      continueGrant.access_token.value
    );
    await redisSet(
      "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN_MANAGE_URL",
      continueGrant.access_token.manage
    );
    console.log("Access Token Manage URL:", continueGrant.access_token.manage);
    console.log("Expires In:", continueGrant.access_token.expires_in);
  } else {
    console.log(
      "Unable to retrieve access token information from continueGrant"
    );
    res.send("Unable to retrieve access token information from continueGrant");
  }

  res.send("Interaction reference and payment token received and stored.");
});

// This endpoint handles the interaction reference and payment token
// It stores the interaction reference in Redis and continues the outgoing payment grant
// The flow: Store interaction reference -> Continue grant -> Store access token
// Error handling is implemented to catch and report any issues during the process
app.get("/auth/outgoing/eur", async (req, res) => {
  const grant_outgoingPayment = await createOutgoingPaymentGrant(
    client,
    eur,
    { assetCode: "EUR", assetScale: 2, value: "100000" },
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

  // Redirect the user to the manage URL for authorization
  res.redirect(grant_outgoingPayment.interact.redirect);
});

// This endpoint handles the creation of outgoing payments
// It processes the payment request, creates a quote, initiates the payment,
// and rotates the access token for security
// The flow: Create quote -> Create outgoing payment -> Rotate access token
// Error handling is implemented to catch and report any issues during the process
app.post("/payment/outgoing/eur", async (req, res) => {
  try {
    // Extract data from the request body
    const { action, userId, amount } = req.body;
    if (action !== "send" || !userId || !amount) {
      // userId not really required for MVP stuff
      return res.status(400).json({ error: "Invalid request body" });
    }
    console.log(`Preparing to send ${amount} EUR for user ${userId}`);

    // GET WALLET ADDRESS
    const walletAddress = await getWalletAddress(client, eur);

    // PAYMENT REQUEST
    const grant_incomingPayment = await createIncomingPaymentGrant(client, eur);
    const paymentRequest = await client.incomingPayment.create(
      {
        url: walletAddress.resourceServer,
        accessToken: grant_incomingPayment.access_token.value,
      },
      {
        walletAddress: eur,
        incomingAmount: {
          value: (amount * 100).toString(),
          assetCode: "EUR",
          assetScale: 2,
        },
        expiresAt: new Date(Date.now() + 60_000 * 10).toISOString(),
      }
    );

    // CREATE QUOTE
    const grant_quote = await createQuoteGrant(client, eur);
    const quote = await client.quote.create(
      {
        url: walletAddress.resourceServer,
        accessToken: grant_quote.access_token.value,
      },
      {
        method: "ilp",
        walletAddress: eur,
        receiver: paymentRequest.id,
      }
    );

    console.log("Quote Response:", quote);

    // CREATE OUTGOING PAYMENT
    const outgoingPayment = await createOutgoingPayment(
      client,
      await redisGet("EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN"),
      walletAddress,
      quote.id
    );

    console.log("Outgoing Payment Response:", outgoingPayment);

    // ROTATE ACCESS TOKEN
    const rotatedToken = await rotateAccessToken(
      client,
      await redisGet("EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN_MANAGE_URL"),
      await redisGet("EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN")
    );

    console.log("Rotated Token Response:", rotatedToken);

    // Update the access token in Redis with the rotated token
    await redisSet(
      "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN_MANAGE_URL",
      rotatedToken.access_token.manage
    );
    await redisSet(
      "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN",
      rotatedToken.access_token.value
    );

    res.json({
      message: "Outgoing payment created successfully",
      paymentDetails: outgoingPayment,
    });
  } catch (error) {
    console.error("Error creating outgoing payment:", error);
    res.status(500).json({ error: "Failed to create outgoing payment" });
  }
});

// This endpoint fetches the balance for a given user
// It constructs the appropriate URL using the provided userId
// Sends a fetch request to the wallet server and returns the balance data
app.get("/balance/eur", async (req, res) => {
  const userId = req.query.userId;
  if (!userId) {
    // not really required for MVP stuff
    return res.status(400).json({ error: "userId is required" });
  }

  const walletUrl = `https://wallet.interledger-test.dev/_next/data/T7-4_x5MO9bq8wKjHZj-b/account/${userId}.json?accountId=${userId}`;
  try {
    const response = await fetch(
      "https://wallet.interledger-test.dev/_next/data/T7-4_x5MO9bq8wKjHZj-b/account/121494fc-575f-4f2c-a25a-f4e56cf414bc.json?accountId=121494fc-575f-4f2c-a25a-f4e56cf414bc",
      {
        headers: {
          accept: "*/*",
          "accept-language": "en-GB,en-US;q=0.9,en;q=0.8",
          "cache-control": "no-cache",
          cookie:
            "testnet.cookie=Fe26.2*1*53ffce86e3437a8f844ece1e4428da7d398d33e6a5172a5c3fc6237006f79c0e*oaHupnwOHsPcNgd443YXiQ*GamCTn9ySXoyjOTS-q0eU9rkI7dRF8cfgzC5TyQ-3BYRaxV0c2ic_wYTxs02uWd3wXGBltcvCdGM2lVp8v9x2CU7HQ-bAD4JDSz5L9QStkAuz-um3tkqWWKDTjOm0c4R1630F-nZzUf_3dNxiTJj1RtdZsDX9uLGJ-lQGRtvfcyFgMuxIltsDQa6HhAKQFwR2EUrIwJbHUtalwsKnIDjOocX_u2xnCU6maTkhlh33i7ttQlmZ-R7DC7WqOs3S_Sw*1731990091228*ab0b3d16baf3607d38e509d151189462b051f24716775eff6331d494b5922073*53UzSyRXdPnUrYmmlRqUzojkOLpTCkUIMD0YAKOwSQk~2",
          dnt: "1",
          pragma: "no-cache",
          priority: "u=1, i",
          referer: "https://wallet.interledger-test.dev/",
          "sec-ch-ua": '"Not?A_Brand";v="99", "Chromium";v="130"',
          "sec-ch-ua-mobile": "?0",
          "sec-ch-ua-platform": '"macOS"',
          "sec-fetch-dest": "empty",
          "sec-fetch-mode": "cors",
          "sec-fetch-site": "same-origin",
          "user-agent":
            "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130.0.0.0 Safari/537.36",
          "x-nextjs-data": "1",
        },
      }
    );

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();
    res.json(data);
  } catch (error) {
    console.error("Error fetching balance:", error);
    res.status(500).json({ error: "Failed to fetch balance" });
  }
});

// Start the server
app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
