plugins {
    java
    application
}

version = "1.0"

application {
    mainClass.set("ex4.Coordinator")
}

java {
    sourceCompatibility = JavaVersion.VERSION_21
    targetCompatibility = JavaVersion.VERSION_21
}

repositories {
    mavenCentral()
    maven("https://repo.eclipse.org/content/repositories/paho-snapshots/")
}

dependencies {
    testImplementation("org.junit.jupiter:junit-jupiter-api:5.5.2")
    testRuntimeOnly("org.junit.jupiter:junit-jupiter-engine:5.5.2")
    testImplementation("org.hamcrest:hamcrest-library:2.1")
    implementation("org.eclipse.paho:org.eclipse.paho.client.mqttv3:1.2.5")
}

tasks.test {
    useJUnitPlatform()
}

tasks.register<Zip>("mciSrcZip") {
    archiveFileName.set("Assignment.zip")
    from(".") {
        include("src/**/*", "*.gradle.kts", "gradlew", "gradlew.bat", "gradle/**/*")
        exclude(".gradle/**", "build/**")
    }
}

tasks.withType<JavaExec> {
    if (System.getProperty("DEBUG", "false") == "true") {
        jvmArgs("-Xdebug", "-Xrunjdwp:transport=dt_socket,server=y,suspend=y,address=9099")
    }
}

tasks.jar {
    duplicatesStrategy = DuplicatesStrategy.INCLUDE
    manifest {
        attributes(
            "Class-Path" to configurations.runtimeClasspath.get().joinToString(" ") { it.name },
            "Main-Class" to "ex4.Coordinator"
        )
    }
    from(configurations.runtimeClasspath.get().map { if (it.isDirectory) it else zipTree(it) })
}
