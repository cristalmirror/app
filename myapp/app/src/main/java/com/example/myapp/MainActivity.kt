package com.example.myapp

import android.net.Uri
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.provider.OpenableColumns
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.Keep
import java.io.File
import java.io.FileOutputStream

class MainActivity : AppCompatActivity() {

    // Declarar funciones nativas
    private external fun connectNative(ip: String, port: Int): Int
    private external fun sendNative(mensaje: String): Int
    private external fun disconnectNative()
    private external fun encryptFile(filePath: String): Int
    private lateinit var txtFileSelected: TextView

    @Keep
    fun updateUIStatus(message: String) {
        runOnUiThread {
            txtFileSelected.text = message
        }
    }

    private val getFileContent = registerForActivityResult(ActivityResultContracts.GetContent()) { uri: Uri? ->
        uri?.let {
            val filePath = copyFileToInternalStorage(it)
            if (filePath != null) {
                txtFileSelected.text = "Archivo: $filePath"
                Toast.makeText(this, "Cifrando archivo...", Toast.LENGTH_SHORT).show()
                
                Thread {
                    val resultado = encryptFile(filePath)
                    runOnUiThread {
                        if (resultado == 0) {
                            Toast.makeText(this, "Archivo cifrado con éxito", Toast.LENGTH_LONG).show()
                        } else {
                            Toast.makeText(this, "Error al cifrar: $resultado", Toast.LENGTH_SHORT).show()
                        }
                    }
                }.start()
            } else {
                Toast.makeText(this, "Error al procesar el archivo", Toast.LENGTH_SHORT).show()
            }
        }
    }

    private fun copyFileToInternalStorage(uri: Uri): String? {
        val returnCursor = contentResolver.query(uri, null, null, null, null)
        val nameIndex = returnCursor?.getColumnIndex(OpenableColumns.DISPLAY_NAME)
        returnCursor?.moveToFirst()
        val name = returnCursor?.getString(nameIndex ?: 0)
        returnCursor?.close()

        val file = File(filesDir, name ?: "temp_file")
        try {
            val inputStream = contentResolver.openInputStream(uri)
            val outputStream = FileOutputStream(file)
            val buffer = ByteArray(1024)
            var read: Int
            while (inputStream?.read(buffer).also { read = it ?: -1 } != -1) {
                outputStream.write(buffer, 0, read)
            }
            inputStream?.close()
            outputStream.close()
            return file.absolutePath
        } catch (e: Exception) {
            e.printStackTrace()
            return null
        }
    }

    private fun searchFiles() {
        getFileContent.launch("*/*")
    }

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
        val btnFind = findViewById<Button>(R.id.btnFind)
        txtFileSelected = findViewById(R.id.txtFileSelected)

        btnRefrescar.setOnClickListener {
            intentarConnect()
        }

        btnFind.setOnClickListener {
            searchFiles()
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
