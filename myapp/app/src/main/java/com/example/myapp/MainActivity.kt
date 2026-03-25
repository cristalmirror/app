package com.example.myapp

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.Toast

class MainActivity : AppCompatActivity() {

    // Declarar funciones nativas
    private external fun connectNative(ip: String, port: Int): Int
    private external fun sendNative(mensaje: String): Int
    private external fun disconnectNative()

    private fun intentarConnect() {
        Thread {
            val resultado = connectNative("192.168.0.51", 1234)
            runOnUiThread {
                if (resultado == 0) {
                    Toast.makeText(this, "Conectado al servidor", Toast.LENGTH_SHORT).show()
                } else {
                    Toast.makeText(this, "Error de conexión: $resultado", Toast.LENGTH_SHORT).show()
                }
            }
        }.start()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val btnEnviar = findViewById<Button>(R.id.btnEnviar)
        val editText = findViewById<EditText>(R.id.editTextMensaje)
        val btnRefrescar = findViewById<Button>(R.id.btnRefrescar)

        btnRefrescar.setOnClickListener {
            intentarConnect()
        }

        // Conectar al servidor al iniciar la app
        intentarConnect()

        btnEnviar.setOnClickListener {
            val texto = editText.text.toString()

            Thread {
                val resultado = sendNative(texto)
                runOnUiThread {
                    if (resultado == 0) {
                        Toast.makeText(this, "Enviado con éxito", Toast.LENGTH_SHORT).show()
                        editText.text.clear()
                    } else {
                        Toast.makeText(this, "Error al enviar: $resultado", Toast.LENGTH_SHORT).show()
                    }
                }
            }.start()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        // Cerrar la conexión cuando la actividad se destruye
        Thread {
            disconnectNative()
        }.start()
    }

    companion object {
        init {
            System.loadLibrary("myapp")
        }
    }
}
