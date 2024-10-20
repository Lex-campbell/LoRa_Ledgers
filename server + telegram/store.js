// This file simulates a simple key-value store using a JSON file for persistence.
// It provides functions to read and write data, mimicking basic Redis-like functionality.
// The data is stored in a local file named 'persistentData.json'.

// Key features:
// 1. Loads existing data from the file on startup
// 2. Provides 'redisSet' function to update values and save to file
// 3. Provides 'redisGet' function to retrieve values (implementation not shown)
// 4. Handles file read/write errors
// 5. Initializes with default values if the file doesn't exist or is corrupted

// This approach allows for persistent storage between program runs without
// requiring an actual Redis server, suitable for development or simple applications.

import fs from "fs";
import path from "path";

const dataFilePath = path.join(process.cwd(), "persistentData.json");

// Initialize data object
let data = {};

// Load existing data from file if it exists
if (fs.existsSync(dataFilePath)) {
  try {
    const fileContent = fs.readFileSync(dataFilePath, "utf8");
    data = JSON.parse(fileContent);
  } catch (error) {
    console.error("Error reading or parsing data file:", error);
    // Initialize with default values if parsing fails
    data = {
      INCOMING_PAYMENT_ACCESS_TOKEN: "",
      INCOMING_PAYMENT_ACCESS_TOKEN_MANAGE_URL: "",
      QUOTE_ACCESS_TOKEN: "",
      QUOTE_ACCESS_TOKEN_MANAGE_URL: "",
      EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN: "",
      EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN_MANAGE_URL: "",
      EUR_OUTGOING_PAYMENT_GRANT_CONTINUE_URI: "",
    };
  }
} else {
  // Set initial values if file doesn't exist
  data = {
    INCOMING_PAYMENT_ACCESS_TOKEN: "",
    INCOMING_PAYMENT_ACCESS_TOKEN_MANAGE_URL: "",
    QUOTE_ACCESS_TOKEN: "",
    QUOTE_ACCESS_TOKEN_MANAGE_URL: "",
    EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN: "",
    EUR_OUTGOING_PAYMENT_GRANT_ACCESS_TOKEN_MANAGE_URL: "",
    EUR_OUTGOING_PAYMENT_GRANT_CONTINUE_URI: "",
  };
}

// Function to update values
const redisSet = (key, value) => {
  return new Promise((resolve, reject) => {
    data[key] = value;
    const jsonString = JSON.stringify(data, null, 2);

    fs.writeFile(dataFilePath, jsonString, "utf8", (err) => {
      if (err) {
        console.error(`Error updating ${key}:`, err);
        reject(err);
      } else {
        console.log(`${key} updated successfully`);
        resolve();
      }
    });
  });
};

// Function to retrieve values
const redisGet = (key) => {
  return new Promise((resolve, reject) => {
    if (key in data) {
      resolve(data[key]);
    } else {
      reject(new Error(`Key "${key}" not found`));
    }
  });
};

export { redisSet, redisGet };
