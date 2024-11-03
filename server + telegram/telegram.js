// This file contains a simple implementation of a Telegram bot for Open Payments
// It is not the core offering, but rather a demonstration of an alternative interface
// The bot shows how live data updates can be accessed through a chat interface
// It uses basic commands to interact with the Open Payments system
// This serves as a proof of concept for integrating Open Payments with messaging platforms

import TelegramBot from "node-telegram-bot-api";
import fetch from "node-fetch";

const bot = new TelegramBot("7520876386:AAGi1FeV9XC6wdHZyc6EVpvuRW7DPHZBgjU", {
  polling: true,
});

const API_BASE_URL = "http://localhost:3344"; // Assuming the local server runs on port 3344

// Command to start the bot
bot.onText(/\/start/, (msg) => {
  const chatId = msg.chat.id;
  bot.sendMessage(
    chatId,
    "Welcome to the Open Payments Bot! Use /help to see available commands."
  );
});

// Help command
bot.onText(/\/help/, (msg) => {
  const chatId = msg.chat.id;
  const helpMessage = `
Available commands:
/balance - Check your EUR balance
/send <amount> - Send EUR to the bot's wallet
/auth - Authorize outgoing payments
  `;
  bot.sendMessage(chatId, helpMessage);
});

// Balance command
bot.onText(/\/balance/, async (msg) => {
  const chatId = msg.chat.id;
  const userId = 1; // msg.from.id.toString();

  try {
    const response = await fetch(
      `${API_BASE_URL}/balance/eur?userId=${userId}`
    );

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();
    const balance = data.pageProps.account.balance / 100;
    bot.sendMessage(chatId, `Your current balance is: ${balance} EUR`);
  } catch (error) {
    console.error("Error fetching balance:", error);
    bot.sendMessage(chatId, "Failed to fetch balance. Please try again later.");
  }
});

// Send command
bot.onText(/\/send (.+)/, async (msg, match) => {
  const chatId = msg.chat.id;
  const amount = match[1];
  const userId = msg.from.id.toString();

  try {
    const response = await fetch(`${API_BASE_URL}/payment/outgoing/eur`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        action: "send",
        userId: userId,
        amount: amount,
      }),
    });

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const result = await response.json();
    bot.sendMessage(chatId, `Payment of ${amount} EUR sent successfully!`);
  } catch (error) {
    console.error("Error creating outgoing payment:", error);
    bot.sendMessage(chatId, "Failed to send payment. Please try again later.");
  }
});

// Auth command
bot.onText(/\/auth/, async (msg) => {
  const chatId = msg.chat.id;

  try {
    const response = await fetch(`${API_BASE_URL}/auth/outgoing/eur`);

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const result = await response.json();
    bot.sendMessage(
      chatId,
      `Please authorize the payment by visiting this URL: ${result.redirectUrl}`
    );
  } catch (error) {
    console.error("Error creating outgoing payment grant:", error);
    bot.sendMessage(
      chatId,
      "Failed to create authorization. Please try again later."
    );
  }
});

// Handle callback_query (for future interactive buttons)
bot.on("callback_query", (callbackQuery) => {
  const action = callbackQuery.data;
  const msg = callbackQuery.message;
  const chatId = msg.chat.id;

  // Handle different actions here
});

console.log("Telegram bot is running...");
