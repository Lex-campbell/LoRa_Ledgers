import { createClient, rotateAccessToken, eur, privateKey } from "./lib.js";
import { redisGet } from "./store.js";

const client = await createClient(
  eur,
  privateKey,
  "ab6842e7-0076-4236-8b0c-4c27aa9dc78c"
);

const accessTokenManageUrl =
  "https://auth.interledger-test.dev/token/5e94a41c-0518-4c52-96fb-44f814142b0e";
const accessToken = "1C977B7299E185EC5FF1";
const continueUri = await redisGet("EUR_OUTGOING_PAYMENT_GRANT_CONTINUE_URI");
const interactionReference = await redisGet("INTERACTION_REFERENCE");

const rotatedToken = await rotateAccessToken(
  client,
  accessTokenManageUrl,
  accessToken
);

console.log("Rotated Token Response:", rotatedToken);

// CONTINUE_GRANT = {
//   access_token: {
//     access: [[Object]],
//     value: "1C977B7299E185EC5FF1",
//     manage:
//       "https://auth.interledger-test.dev/token/5e94a41c-0518-4c52-96fb-44f814142b0e",
//     expires_in: 600,
//   },
//   continue: {
//     access_token: { value: "9445ECDE792ECD43278B" },
//     uri: "https://auth.interledger-test.dev/continue/5cfbc42a-b358-49b3-9adb-dd469b31d9ad",
//   },
// };
