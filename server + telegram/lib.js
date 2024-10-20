import {
  createAuthenticatedClient,
  isPendingGrant,
} from "@interledger/open-payments";
import fs from "fs";
import path from "path";
import { redisGet, redisSet } from "./store";

// Load private key for authentication
export const privateKeyPath = path.join(process.cwd(), "private.key");
export const privateKey = fs.readFileSync(privateKeyPath, "utf8");

// Sender and receiver wallets
export const eur = "https://ilp.interledger-test.dev/7a641094"; // Sender (EUR)
export const zar = "https://ilp.interledger-test.dev/yolo2"; // Receiver (ZAR)

// Helper function to load private key
export const loadPrivateKey = (keyPath) => {
  return fs.readFileSync(path.join(process.cwd(), keyPath), "utf8");
};

// Function to handle token rotation timer
// export const startTokenRotationTimer = (client) => {
//   const rotateToken = async () => {
//     try {
//       const rotatedToken = await rotateAccessToken(
//         client,
//         await redisGet("EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN_MANAGE_URL"),
//         await redisGet("EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN")
//       );

//       console.log("Rotated Token Response:", rotatedToken);

//       // Update the access token in Redis with the rotated token
//       await redisSet(
//         "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN_MANAGE_URL",
//         rotatedToken.access_token.manage
//       );
//       await redisSet(
//         "EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN",
//         rotatedToken.access_token.value
//       );
//     } catch (error) {
//       console.error("Failed to rotate access token:", error);
//     }
//   };

//   // Initial rotation
//   // rotateToken();

//   // Set up interval for rotation every 30 seconds
//   const intervalId = setInterval(rotateToken, 30000);

//   // Return a function to stop the timer if needed
//   return () => clearInterval(intervalId);
// };

// Helper function to create authenticated client
export const createClient = async (walletAddressUrl, privateKey, keyId) => {
  return await createAuthenticatedClient({
    walletAddressUrl,
    privateKey,
    keyId,
    logLevel: "debug",
  });
};

// Helper function to get wallet address
export const getWalletAddress = async (client, walletUrl) => {
  return await client.walletAddress.get({ url: walletUrl });
};

// Helper function to request grant
export const createIncomingPaymentGrant = async (client, walletUrl) => {
  const walletAddress = await getWalletAddress(client, walletUrl);

  const grant = await client.grant.request(
    { url: walletAddress.authServer },
    {
      access_token: {
        access: [
          {
            type: "incoming-payment",
            actions: ["list", "read", "complete", "create"],
          },
        ],
      },
      client: walletAddress.id,
    }
  );

  if (isPendingGrant(grant)) {
    throw new Error("Expected non-interactive grant");
  }

  console.log("INCOMING_PAYMENT_ACCESS_TOKEN =", grant.access_token.value);
  console.log(
    "INCOMING_PAYMENT_ACCESS_TOKEN_MANAGE_URL = ",
    grant.access_token.manage
  );

  return grant;
};

// Helper function to request quote grant
export const createQuoteGrant = async (client, walletUrl) => {
  const walletAddress = await getWalletAddress(client, walletUrl);

  const grant = await client.grant.request(
    { url: walletAddress.authServer },
    {
      access_token: {
        access: [
          {
            type: "quote",
            actions: ["create", "read", "read-all"],
          },
        ],
      },
      client: walletAddress.id,
    }
  );

  if (isPendingGrant(grant)) {
    throw new Error("Expected non-interactive grant");
  }

  console.log("QUOTE_ACCESS_TOKEN =", grant.access_token.value);
  console.log("QUOTE_ACCESS_TOKEN_MANAGE_URL = ", grant.access_token.manage);

  return grant;
};

// Helper function to request outgoing payment grant
export const createOutgoingPaymentGrant = async (
  client,
  walletUrl,
  //   debitAmount,
  receiveAmount,
  nonce
) => {
  const walletAddress = await getWalletAddress(client, walletUrl);

  const grant = await client.grant.request(
    { url: walletAddress.authServer },
    {
      access_token: {
        access: [
          {
            identifier: walletAddress.id,
            type: "outgoing-payment",
            actions: ["list", "read", "create"],
            limits: {
              //   debitAmount: {
              //     assetCode: debitAmount.assetCode,
              //     assetScale: debitAmount.assetScale,
              //     value: debitAmount.value,
              //   },
              receiveAmount: {
                assetCode: receiveAmount.assetCode,
                assetScale: receiveAmount.assetScale,
                value: receiveAmount.value,
              },
              interval: "R/2024-10-19T08:00:00Z/P1D",
            },
          },
        ],
      },
      interact: {
        start: ["redirect"],
        finish: {
          method: "redirect",
          uri: "http://localhost:3344",
          nonce: nonce,
        },
      },
    }
  );

  return grant;
};

// Helper function to continue outgoing payment grant
export const continueOutgoingPaymentGrant = async (
  client,
  continueAccessToken,
  continueUri,
  interactRef
) => {
  const grant = await client.grant.continue(
    {
      accessToken: continueAccessToken,
      url: continueUri,
    },
    {
      interact_ref: interactRef,
    }
  );

  return grant;
};

// Helper function to rotate access token
export const rotateAccessToken = async (client, manageUrl, accessToken) => {
  const token = await client.token.rotate({
    url: manageUrl,
    accessToken: accessToken,
  });
  return token;
};

// Helper function to create incoming payment
export const createIncomingPayment = async (
  client,
  grant,
  amount,
  assetCode,
  assetScale
) => {
  const walletAddress = await getWalletAddress(client);
  return await client.incomingPayment.create(
    {
      url: walletAddress.resourceServer,
      accessToken: grant.access_token.value,
    },
    {
      walletAddress: walletAddress.id,
      incomingAmount: {
        value: amount,
        assetCode,
        assetScale,
      },
      expiresAt: new Date(Date.now() + 365 * 24 * 60 * 60 * 1000).toISOString(),
    }
  );
};

// Helper function to create outgoing payment
export const createOutgoingPayment = async (
  client,
  paymentToken,
  walletAddress,
  quoteId
) => {
  console.log("Creating outgoing payment with token:", paymentToken);
  return await client.outgoingPayment.create(
    {
      url: walletAddress.resourceServer,
      accessToken: paymentToken,
    },
    {
      walletAddress: walletAddress.id,
      quoteId: quoteId,
    }
  );
};

// Helper function to create quote
export const createQuote = async (client, grant, incomingPayment) => {
  const walletAddress = await getWalletAddress(client);
  return await client.quote.create(
    {
      url: walletAddress.resourceServer,
      accessToken: grant.access_token.value,
    },
    {
      method: "ilp",
      walletAddress: walletAddress.id,
      receiver: incomingPayment.id,
    }
  );
};
